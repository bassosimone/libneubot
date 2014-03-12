/* libneubot/httpclient.c */

/*-
 * Copyright (c) 2014
 *     Nexa Center for Internet & Society, Politecnico di Torino (DAUIN)
 *     and Simone Basso <bassosimone@gmail.com>.
 *
 * This file is part of Neubot <http://www.neubot.org/>.
 *
 * Neubot is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Neubot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Neubot.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * HTTP client
 */

#include <sys/types.h>
#include <sys/queue.h>

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/keyvalq_struct.h>

#include "http-parser/http_parser.h"

#include "neubot.h"

#include "log.h"
#include "poller.h"
#include "strtonum.h"
#include "utils.h"

struct NeubotHttpClient {
	struct bufferevent *bev;
	size_t body_len;
	const char *body_ptr;
	char *body_string;		// XXX: possible leak in cleanup
	neubot_hook_vo handle_begin;
	neubot_hook_vo handle_body;
	neubot_hook_vo handle_connect;
	neubot_hook_vo handle_end;
	neubot_hook_vo handle_flush;
	neubot_hook_vo handle_headers;
	neubot_hook_vo handle_close;
	struct evbuffer *header_key;
	struct evbuffer *header_value;
	struct evkeyvalq headers;
	int last_caller;
	void *opaque;
	struct http_parser parser;
	struct NeubotPoller *poller;
	struct evbuffer *readbuf;
	struct evbuffer *reason;
	struct http_parser_settings settings;
	unsigned int state;
};

#define HEADERS_CALLER_NOTHING 0
#define HEADERS_CALLER_KEY 1
#define HEADERS_CALLER_VALUE 2
#define HEADERS_CALLER_EOH 3

#define STATE_INITIAL 0
#define STATE_CONNECTING (1<<0)
#define STATE_CONNECTED (1<<1)
#define STATE_WRITING (1<<2)
#define STATE_PARSER (1<<3)
#define STATE_VALID_META (1<<4)
#define STATE_CLOSING (1<<5)

#define EVBUFFER_CLEAR_(__buf)                                          \
	do {                                                            \
		int result;                                             \
		size_t total;                                           \
                                                                        \
		total = evbuffer_get_length(__buf);                     \
		result = evbuffer_drain(__buf, total);                  \
		if (result != 0)                                        \
			return (-1);                                    \
	} while (0)

static int
NeubotHttpClient_on_message_begin(struct http_parser *parser)
{
	struct NeubotHttpClient *self;
	self = (struct NeubotHttpClient *) parser->data;

	EVBUFFER_CLEAR_(self->header_key);
	EVBUFFER_CLEAR_(self->header_value);
	EVBUFFER_CLEAR_(self->reason);
	evhttp_clear_headers(&self->headers);
	self->last_caller = HEADERS_CALLER_NOTHING;
	self->body_string = NULL;
	self->body_ptr = NULL;
	self->body_len = 0;

	self->handle_begin(self->opaque);

	return (0);
}

#define EVBUFFER_ADD_(__buf, __data, __len)                             \
	do {                                                            \
		int result;                                             \
                                                                        \
		if (__data == NULL || __len <= 0)                       \
			break;                                          \
                                                                        \
		result = evbuffer_add(__buf, __data, __len);            \
		if (result != 0)                                        \
			return (-1);                                    \
	} while (0)

static int
NeubotHttpClient_on_url(struct http_parser *parser,
    const char *data, size_t len)
{
	return (-1);  /* Unexpected */
}

static int
NeubotHttpClient_on_status(struct http_parser *parser,
    const char *data, size_t len)
{
	struct NeubotHttpClient *self;
	self = (struct NeubotHttpClient *) parser->data;

	EVBUFFER_ADD_(self->reason, data, len);

	return (0);
}

static inline int
NeubotHttpClient_headers_fsm(struct NeubotHttpClient *self, int caller,
    const char *data, size_t len)
{
	struct evbuffer *evbuf;
	int result, needsave;

	if (self->last_caller == HEADERS_CALLER_NOTHING &&
	    caller == HEADERS_CALLER_KEY) {
		evbuf = self->header_key;
		needsave = 0;

	} else if (self->last_caller == HEADERS_CALLER_VALUE &&
	    caller == HEADERS_CALLER_KEY) {
		evbuf = self->header_key;
		needsave = 1;

	} else if (self->last_caller == HEADERS_CALLER_KEY &&
	    caller == HEADERS_CALLER_KEY) {
		evbuf = self->header_key;
		needsave = 0;

	} else if (self->last_caller == HEADERS_CALLER_KEY &&
	    caller == HEADERS_CALLER_VALUE) {
		evbuf = self->header_value;
		needsave = 0;

	} else if (self->last_caller == HEADERS_CALLER_VALUE &&
	    caller == HEADERS_CALLER_VALUE) {
		evbuf = self->header_value;
		needsave = 0;

	} else if (self->last_caller == HEADERS_CALLER_VALUE &&
	    caller == HEADERS_CALLER_EOH) {
		evbuf = NULL;
		needsave = 1;

	} else
		abort();

	if (needsave) {
		char *key, *value;

		/* Terminate the strings */
		EVBUFFER_ADD_(self->header_key, "\0", 1);
		EVBUFFER_ADD_(self->header_value, "\0", 1);

		key = (char *) evbuffer_pullup(self->header_key, -1);
		if (key == NULL)
			return (-1);
		value = (char *) evbuffer_pullup(self->header_value, -1);
		if (value == NULL)
			return (-1);
		result = evhttp_add_header(&self->headers, key, value);
		if (result != 0)
			return (-1);

		/* Start over */
		EVBUFFER_CLEAR_(self->header_key);
		EVBUFFER_CLEAR_(self->header_value);
	}

	if (evbuf != NULL)
		EVBUFFER_ADD_(evbuf, data, len);

	self->last_caller = caller;

	return (0);
}

static int
NeubotHttpClient_on_header_field(struct http_parser *parser,
    const char *data, size_t len)
{
	struct NeubotHttpClient *self;
	int result;

	self = (struct NeubotHttpClient *) parser->data;

	result = NeubotHttpClient_headers_fsm(self,
	    HEADERS_CALLER_KEY, data, len);
	if (result != 0)
		return (-1);

	return (0);
}

static int
NeubotHttpClient_on_header_value(struct http_parser *parser,
    const char *data, size_t len)
{
	struct NeubotHttpClient *self;
	int result;

	self = (struct NeubotHttpClient *) parser->data;

	result = NeubotHttpClient_headers_fsm(self,
	    HEADERS_CALLER_VALUE, data, len);
	if (result != 0)
		return (-1);

	return (0);
}

static int
NeubotHttpClient_on_headers_complete(struct http_parser *parser)
{
	struct NeubotHttpClient *self;
	int result;

	self = (struct NeubotHttpClient *) parser->data;

	result = NeubotHttpClient_headers_fsm(self,
	    HEADERS_CALLER_EOH, NULL, 0);
	if (result != 0)
		return (-1);

	// Terminate the string
	EVBUFFER_ADD_(self->reason, "\0", 1);

	/* Tell the getter methods that now metadata is valid */
	self->state |= STATE_VALID_META;
	self->handle_headers(self->opaque);
	self->state &= ~STATE_VALID_META;

	return (0);
}

static int
NeubotHttpClient_on_body(struct http_parser *parser,
    const char *data, size_t len)
{
	struct NeubotHttpClient *self;
	self = (struct NeubotHttpClient *) parser->data;

	/*
	 * Do not copy the body, because the callee may not be
	 * interested (e.g., the callee may only want to measure
	 * but may not be interested into the body).
	 */
	self->body_string = NULL;
	self->body_ptr = data;
	self->body_len = len;

	self->handle_body(self->opaque);

	// Clear the copy of the current body piece (if any)
	if (self->body_string != NULL)
		free(self->body_string);

	self->body_string = NULL;
	self->body_ptr = NULL;
	self->body_len = 0;

	return (0);
}

static int
NeubotHttpClient_on_message_complete(struct http_parser *parser)
{
	struct NeubotHttpClient *self;

	self = (struct NeubotHttpClient *) parser->data;
	self->handle_end(self->opaque);

	return (0);
}

static int
NeubotHttpClient_parser_execute_(struct NeubotHttpClient *self,
    const char *data, size_t count)
{
	ssize_t n;

	/*
	 * Hint for close(): we cannot shut down abruptly because we
	 * are parsing the incoming data.
	 */
	self->state |= STATE_PARSER;
	n = http_parser_execute(&self->parser, &self->settings, data, count);
	self->state &= ~STATE_PARSER;

	if (self->parser.upgrade || n != count)
		return (-1);

	/*
	 * Fail if the connection is not established anymore because the
	 * user called close() while we were parsing.
	 */
	if ((self->state & STATE_CONNECTED) == 0)
		return (-1);

	return (n);
}

static inline void
NeubotHttpClient_close_(struct NeubotHttpClient *self)
{
	if (self != NULL) {
		self->handle_close(self->opaque);
		if (self->bev != NULL)
			bufferevent_free(self->bev);
		if (self->header_key != NULL)
			evbuffer_free(self->header_key);
		if (self->header_value != NULL)
			evbuffer_free(self->header_value);
		if (self->readbuf != NULL)
			evbuffer_free(self->readbuf);
		if (self->reason != NULL)
			evbuffer_free(self->reason);
	}
	neubot_xfree(self);
}

static void
NeubotHttpClient_bufferevent_read(struct bufferevent *bev, void *opaque)
{
	struct evbuffer_iovec iovector[32];
	struct NeubotHttpClient *self;
	int result, count, total;

	self = (struct NeubotHttpClient *) opaque;

	if ((self->state & STATE_CONNECTED) == 0)
		abort();

	result = bufferevent_read_buffer(bev, self->readbuf);
	if (result != 0)
		goto fail;

	total = evbuffer_peek(self->readbuf, -1, NULL, iovector, 32);
	if (total < 0 || total > sizeof (iovector))
		abort();

	for (count = 0; count < total; ++count) {
		ssize_t parsed;

		parsed = NeubotHttpClient_parser_execute_(self,
		    iovector[count].iov_base, iovector[count].iov_len);
		if (parsed == -1)
			goto fail;

		result = evbuffer_drain(self->readbuf, parsed);
		if (result != 0)
			goto fail;
	}

	return;  /* Success */

fail:	NeubotHttpClient_close_(self);
}

static void
NeubotHttpClient_bufferevent_write(struct bufferevent *bev, void *opaque)
{
	struct NeubotHttpClient *self;
	self = (struct NeubotHttpClient *) opaque;
	self->handle_flush(self->opaque);
}

static void
NeubotHttpClient_bufferevent_event(struct bufferevent *bev,
    short what, void *opaque)
{
	struct NeubotHttpClient *self;

	self = (struct NeubotHttpClient *) opaque;

	if ((self->state & STATE_CONNECTING) != 0) {
		if ((what & BEV_EVENT_CONNECTED) == 0) {
			NeubotHttpClient_close_(self);
			return;
		}
		self->state &= ~STATE_CONNECTING;
		self->state |= STATE_CONNECTED;
		self->handle_connect(self->opaque);
		return;
	}

	if ((what & (BEV_EVENT_READING|BEV_EVENT_EOF)) != 0)
		(void)NeubotHttpClient_parser_execute_(self, NULL, 0);

	NeubotHttpClient_close_(self);
}

static void
NeubotHttpClient_noop(void *opaque)
{
	/* nothing */ ;
}

#define SAFELY_INIT_CALLBACK(__foo)                                     \
	if (__foo == NULL)                                              \
		__foo = NeubotHttpClient_noop;                          \
	self->__foo = __foo;

struct NeubotHttpClient *
NeubotHttpClient_construct(struct NeubotPoller *poller,
    neubot_slot_vo handle_begin, neubot_slot_vo handle_body,
    neubot_slot_vo handle_close, neubot_slot_vo handle_connect,
    neubot_slot_vo handle_end, neubot_slot_vo handle_flush,
    neubot_slot_vo handle_headers, void *opaque)
{
	struct NeubotHttpClient *self;
	struct event_base *evbase;
	int result;

	self = calloc(1, sizeof (*self));
	if (self == NULL)
		goto fail;

	evbase = NeubotPoller_event_base_(poller);
	if (evbase == NULL)
		goto fail;

	/* Must initialize first otherwise there may be leaks */
	SAFELY_INIT_CALLBACK(handle_close);
	self->opaque = opaque;

	/* Note: The code assumes that the callbacks are run sequentially */
	self->bev = bufferevent_socket_new(evbase, -1,
	    BEV_OPT_CLOSE_ON_FREE|BEV_OPT_DEFER_CALLBACKS);
	if (self->bev == NULL)
		goto fail;

	SAFELY_INIT_CALLBACK(handle_begin);
	SAFELY_INIT_CALLBACK(handle_body);
	SAFELY_INIT_CALLBACK(handle_connect);
	SAFELY_INIT_CALLBACK(handle_end);
	SAFELY_INIT_CALLBACK(handle_flush);
	SAFELY_INIT_CALLBACK(handle_headers);

	self->header_key = evbuffer_new();
	if (self->header_key == NULL)
		goto fail;

	self->header_value = evbuffer_new();
	if (self->header_value == NULL)
		goto fail;

	TAILQ_INIT(&self->headers);  // XXX: cleanup leak

	http_parser_init(&self->parser, HTTP_RESPONSE);
	self->parser.data = self;

	self->poller = poller;

	self->readbuf = evbuffer_new();
	if (self->readbuf == NULL)
		goto fail;

	self->reason = evbuffer_new();
	if (self->reason == NULL)
		goto fail;

	self->settings.on_message_begin = NeubotHttpClient_on_message_begin;
	self->settings.on_url = NeubotHttpClient_on_url;
	self->settings.on_status = NeubotHttpClient_on_status;
	self->settings.on_header_field = NeubotHttpClient_on_header_field;
	self->settings.on_header_value = NeubotHttpClient_on_header_value;
	self->settings.on_headers_complete =
	    NeubotHttpClient_on_headers_complete;
	self->settings.on_body = NeubotHttpClient_on_body;
	self->settings.on_message_complete =
	    NeubotHttpClient_on_message_complete;

	bufferevent_setcb(self->bev, NeubotHttpClient_bufferevent_read,
	    NeubotHttpClient_bufferevent_write,
	    NeubotHttpClient_bufferevent_event, self);

	result = bufferevent_enable(self->bev, EV_READ);
	if (result != 0)
		goto fail;

	return (self);

fail:	NeubotHttpClient_close_(self);
	return (NULL);
}

int
NeubotHttpClient_connect(struct NeubotHttpClient *self, const char *family,
    const char *address, const char *port)
{
	int _family;
	int _port;
	struct evdns_base *evdns;
	const char *errstr;
	int result;

	if (self->state != STATE_INITIAL)
		return (-1);

	if (strcmp(family, "PF_UNSPEC") == 0)
		_family = PF_UNSPEC;
	else if (strcmp(family, "PF_INET") == 0)
		_family = PF_INET;
	else if (strcmp(family, "PF_INET6") == 0)
		_family = PF_INET6;
	else
		return (-1);

	_port = (int) neubot_strtonum(port, 0, 65535, &errstr);
	if (errstr != NULL)
		return (-1);

	evdns = NeubotPoller_evdns_base_(self->poller);
	if (evdns == NULL)
		return (-1);

	result = bufferevent_socket_connect_hostname(self->bev, evdns,
	    _family, address, _port);
	if (result != 0)
		return (-1);

	self->state |= STATE_CONNECTING;

	return (0);
}

int
NeubotHttpClient_write(struct NeubotHttpClient *self, const char *data,
    size_t count)
{
	if ((self->state & STATE_CONNECTED) == 0)
		return (-1);
	return (bufferevent_write(self->bev, data, count));
}

int
NeubotHttpClient_writes(struct NeubotHttpClient *self, const char *str)
{
	return (NeubotHttpClient_write(self, str, strlen(str)));
}

int
NeubotHttpClient_flush(struct NeubotHttpClient *self)
{
	if ((self->state & STATE_CONNECTED) == 0)
		return (-1);
	return (bufferevent_flush(self->bev, EV_WRITE, BEV_FLUSH));
}

int
NeubotHttpClient_code(struct NeubotHttpClient *self)
{
	if ((self->state & STATE_VALID_META) == 0)
		return (0);
	return (self->parser.status_code);
}

const char *
NeubotHttpClient_reason(struct NeubotHttpClient *self)
{
	char *result;

	if ((self->state & STATE_VALID_META) == 0)
		return (NULL);

	// Assume that the string is zero terminated
	result = (char *) evbuffer_pullup(self->reason, -1);
	if (result == NULL)
		return (NULL);

	return (result);
}

const char *
NeubotHttpClient_header(struct NeubotHttpClient *self, const char *key)
{
	const char *result;

	if ((self->state & STATE_VALID_META) == 0)
		return (NULL);

	result = evhttp_find_header(&self->headers, key);
	if (result == NULL)
		return (NULL);

	return (result);
}

size_t
NeubotHttpClient_body_length(struct NeubotHttpClient *self)
{
	return (self->body_len);
}

const char *
NeubotHttpClient_body_string(struct NeubotHttpClient *self)
{
	if (self->body_ptr == NULL)
		return (NULL);

	if (self->body_len + 1 > SSIZE_MAX)
		return (NULL);

	if (self->body_string != NULL)
		return (self->body_string);

	self->body_string = calloc(1, self->body_len + 1);
	if (self->body_string == NULL)
		return (NULL);

	memcpy(self->body_string, self->body_ptr, self->body_len);

	return (self->body_string);
}

void
NeubotHttpClient_close(struct NeubotHttpClient *self)
{
	if ((self->state & STATE_CLOSING) != 0)
		return;

	self->state |= STATE_CLOSING;

	/* Defer close*() if the HTTP parser is running */
	if ((self->state & STATE_PARSER) != 0) {
		self->state &= ~STATE_CONNECTED;
		return;
	}

	NeubotHttpClient_close_(self);
}

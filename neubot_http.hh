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

#ifndef NEUBOT_HTTP_HH
# define NEUBOT_HTTP_HH

#include <sys/queue.h>

#include <event2/http.h>
#include <event2/keyvalq_struct.h>

#include "http-parser/http_parser.h"
#include "log.h"
#include "neubot.h"
#include "neubot_core.hh"
#include "utils.h"

// Definitions related to headers parsing:
#define CALLER_NOTHING 0
#define CALLER_KEY 1
#define CALLER_VALUE 2
#define CALLER_EOH 3

#ifdef __cplusplus

struct HttpHandler {
	virtual void on_message_begin(void) = 0;
	virtual void on_headers_complete(void) = 0;
	virtual void on_body(Buffer *) = 0;
	virtual void on_message_complete(void) = 0;
	virtual void on_error(void) = 0;
	virtual NeubotPoller *get_poller(void) = 0;
	virtual ~HttpHandler(void);
};


class Http : public Protocol {

	Buffer *body;
	Buffer *buffer;
	HttpHandler *handler;
	struct evkeyvalq *headers;
	Buffer *key;
	http_parser *parser;
	unsigned prev;
	Buffer *reason;
	http_parser_settings *settings;
	Stream *stream;
	Buffer *url;
	Buffer *value;

	//
	// HTTP parser callbacks:
	//

	static int on_message_begin(http_parser *parser) {
		Http *self = (Http *) parser->data;

		neubot_warn("http:on_message_begin - enter");

		if (self->body->clear() != 0 ||
		    self->key->clear() != 0 ||
		    self->value->clear() != 0 ||
		    self->reason->clear() != 0 ||
		    self->url->clear() != 0) {
			neubot_warn("Http: state->*->clear() failed");
			return (-1);
		}

		evhttp_clear_headers(self->headers);
		self->prev = CALLER_NOTHING;

		self->handler->on_message_begin();

		neubot_warn("http:on_message_begin - leave");
		return (0);
	}

	static int on_url(http_parser *parser, const char *data, size_t len) {
		neubot_info("http:on_url - %ld bytes", len);
		Http *self = (Http *) parser->data;
		return (self->url->write(data, len));
	}

	static int on_status(http_parser *parser, const char *data,
	    size_t len) {
		neubot_info("http:on_status - %ld bytes", len);
		Http *self = (Http *) parser->data;
		return (self->reason->write(data, len));
	}

	// Common function for parsing headers
	int headers_fsm(unsigned caller, const char *data, size_t len) {
		Buffer *buff;
		int result, needsave;

		neubot_info("http:headers_fsm - enter");
		neubot_info("http:headers_fsm - bytes=%ld, caller=%u",
		    len, caller);

		//
		// Decide what to do, depending on the current state transition
		//

		if (this->prev == CALLER_NOTHING && caller == CALLER_KEY) {
			buff = this->key;
			needsave = 0;

		} else if (this->prev == CALLER_VALUE && caller == CALLER_KEY) {
			buff = this->key;
			needsave = 1;

		} else if (this->prev == CALLER_KEY && caller == CALLER_KEY) {
			buff = this->key;
			needsave = 0;

		} else if (this->prev == CALLER_KEY && caller == CALLER_VALUE) {
			buff = this->value;
			needsave = 0;

		} else if (this->prev == CALLER_VALUE
		    && caller == CALLER_VALUE) {
			buff = this->value;
			needsave = 0;

		} else if (this->prev == CALLER_VALUE && caller == CALLER_EOH) {
			buff = NULL;
			needsave = 1;

		} else {
			neubot_warn("Http - headers fsm: internal error");
			abort();
		}

		neubot_info("http:headers_fsm: buff=%p, needsave=%d",
		    (void *) buff, needsave);

		//
		// Do the action bound with the current state transition
		//

		if (needsave) {
			neubot_info("http:headers_fsm - needsave enter");

			char *key = this->key->read_string();
			if (key == NULL) {
				neubot_warn(
				    "http:headers_fsm - cannot read key");
				return (-1);
			}

			char *value = this->value->read_string();
			if (value == NULL) {
				neubot_warn(
				    "http:headers_fsm - cannot read value");
				return (-1);
			}

			neubot_info("< %s: %s", key, value);

			result = evhttp_add_header(this->headers, key, value);
			if (result != 0) {
				neubot_warn("Http - cannot add header");
				return (-1);
			}

			neubot_info("http:headers_fsm - needsave leave");
		}

		if (buff != NULL && buff->write(data, len) != 0) {
			neubot_warn("Http - cannot write into buffer");
			return (-1);
		}

		//
		// Complete the transition to the next state
		//

		this->prev = caller;
		neubot_info("http:headers_fsm - leave");
		return (0);
	}

	static int on_header_field(http_parser *parser, const char *data,
	    size_t len) {
		neubot_info("http:on_header_field - %ld bytes", len);
		Http *self = (Http *) parser->data;
		return (self->headers_fsm(CALLER_KEY, data, len));
	}

	static int on_header_value(http_parser *parser, const char *data,
	    size_t len) {
		neubot_info("http:on_header_value - %ld bytes", len);
		Http *self = (Http *) parser->data;
		return (self->headers_fsm(CALLER_VALUE, data, len));
	}

	static int on_headers_complete(http_parser *parser) {
		Http *self = (Http *) parser->data;

		neubot_info("http:on_headers_complete - enter");

		int result = self->headers_fsm(CALLER_EOH, NULL, 0);
		if (result != 0) {
			neubot_warn("Http - headers_fsm() failed");
			return (-1);
		}

		self->handler->on_headers_complete();

		neubot_info("http:on_headers_complete - leave");
		return (0);
	}

	static int on_body(http_parser *parser, const char *data,
	    size_t length) {
		Http *self = (Http *) parser->data;
		int result;

		//
		// TODO: avoid making a copy
		//

		result = self->body->write(data, length);
		if (result != 0) {
			neubot_warn("HttpParser - cannot save body");
			return (-1);
		}

		self->handler->on_body(self->body);

		// TODO: clear ->body?

		return (0);
	}

	static int on_message_complete(http_parser *parser) {
		Http *self = (Http *) parser->data;
		self->handler->on_message_complete();
		return (0);
	}

	//
	// Private constructor:
	//

	Http(void) : Protocol () {
		this->body = NULL;
		this->buffer = NULL;
		this->handler = NULL;
		this->headers = NULL;
		this->key = NULL;
		this->parser = NULL;
		this->prev = CALLER_NOTHING;
		this->reason = NULL;
		this->settings = NULL;
		this->stream = NULL;
		this->url = NULL;
		this->value = NULL;
	}

    public:

	static Http *construct(HttpHandler *handler, long long fileno,
	    unsigned isclient) {
		neubot_info("construct - enter");

		//
		// Sanity
		//

		if (handler == NULL) {
			neubot_warn("construct - invalid handler");
			return (NULL);
		}

		NeubotPoller *poller = handler->get_poller();
		if (poller == NULL) {
			neubot_warn("construct - invalid poller");
			return (NULL);
		}

		// fileno: checked by stream

		//
		// Alloc
		//

		Http *self = new (std::nothrow) Http();
		if (self == NULL) {
			neubot_warn("construct - cannot construct self");
			return (NULL);
		}

		self->body = Buffer::construct(poller);
		if (self->body == NULL) {
			neubot_warn("construct - cannot construct body");
			delete self;
			return (NULL);
		}

		self->buffer = Buffer::construct(poller);
		if (self->buffer == NULL) {
			neubot_warn("construct - cannot construct buffer");
			delete self;
			return (NULL);
		}

		// handler: nothing to do

		self->headers = (evkeyvalq *) calloc(1,
		    sizeof (*self->headers));
		if (self->headers == NULL) {
			neubot_warn("construct - cannot construct headers");
			delete self;
			return (NULL);
		}

		self->key = Buffer::construct(poller);
		if (self->key == NULL) {
			neubot_warn("construct - cannot construct key");
			delete self;
			return (NULL);
		}

		self->parser = (http_parser *) calloc(1,
		    sizeof (*self->parser));
		if (self->parser == NULL) {
			neubot_warn("construct - cannot construct parser");
			delete self;
			return (NULL);
		}

		// prev: nothing to do

		self->reason = Buffer::construct(poller);
		if (self->reason == NULL) {
			neubot_warn("construct - cannot construct reason");
			delete self;
			return (NULL);
		}

		self->settings = (http_parser_settings *) calloc(1,
		    sizeof (*self->settings));
		if (self->settings == NULL) {
			neubot_warn("construct - cannot construct settings");
			delete self;
			return (NULL);
		}

		// stream: is constructed later

		self->url = Buffer::construct(poller);
		if (self->url == NULL) {
			neubot_warn("construct - cannot construct url");
			delete self;
			return (NULL);
		}

		self->value = Buffer::construct(poller);
		if (self->value == NULL) {
			neubot_warn("construct - cannot construct value");
			delete self;
			return (NULL);
		}

		//
		// Init
		//

		self->handler = handler;

		TAILQ_INIT(self->headers);

		http_parser_init(self->parser, (isclient) ? HTTP_RESPONSE
		    : HTTP_REQUEST);
		self->parser->data = self;

		self->settings->on_message_begin = self->on_message_begin;
		self->settings->on_url = self->on_url;
		self->settings->on_status = self->on_status;
		self->settings->on_header_field = self->on_header_field;
		self->settings->on_header_value = self->on_header_value;
		self->settings->on_headers_complete = self->on_headers_complete;
		self->settings->on_body = self->on_body;
		self->settings->on_message_complete = self->on_message_complete;

		//
		// Recursively construct to the stream
		//

		self->stream = Stream::construct(self, fileno);
		if (self->stream == NULL) {
			neubot_warn("construct - cannot construct stream");
			delete self;
			return (NULL);
		}

		//
		// Success
		//

		neubot_info("construct - leave");
		return (self);
	}

	//
	// Implement the `Protocol' interface:
	//

	virtual void on_data(void) {
		neubot_info("http:on_data - enter");

		int result = this->stream->read_buffer(this->buffer);
		if (result != 0) {
			neubot_warn("Http - cannot read buffer");
			this->handler->on_error();
			return;
		}

		evbuffer *evbuf = this->buffer->get_evbuffer();
		if (evbuf == NULL) {
			neubot_warn("Http - cannot get evbuffer");
			this->handler->on_error();
			return;
		}

		//
		// TODO: avoid making a copy
		//

		const char *data = (const char *) evbuffer_pullup(evbuf, -1);
		if (data == NULL) {
			neubot_warn("Http - cannot pullup");
			this->handler->on_error();
			return;
		}

		size_t count = evbuffer_get_length(evbuf);
		if (count > SSIZE_MAX) {
			neubot_warn("Http - too much buffered data");
			this->handler->on_error();
			return;
		}

		neubot_info("http:on_data - %ld bytes to parser", count);

		ssize_t consumed = http_parser_execute(this->parser,
		    this->settings, data, count);

		neubot_info("http:on_data - parser returned %ld", consumed);

		if (this->parser->upgrade) {
			neubot_warn("Http - unexpected UPGRADE");
			this->handler->on_error();
			return;
		}
		if (consumed < 0) {
			neubot_warn("Http - parser error");
			this->handler->on_error();
			return;
		}
		if ((size_t) consumed != count) {
			neubot_warn("Http - not all data was parsed");
			this->handler->on_error();
			return;
		}

		result = evbuffer_drain(evbuf, consumed);
		if (result != 0) {
			neubot_warn("Http - cannot drain buffer");
			this->handler->on_error();
			return;
		}

		neubot_info("http:on_data - leave");
	}

	virtual void on_flush(void) {
		// nothing
	}

	void on_eof(void) {
		this->handler->on_error();  // Inaccurate
	}

	void on_error(void) {
		this->handler->on_error();
	}

	//
	// Accessors:
	//

	NeubotPoller *get_poller(void) {
		return (this->handler->get_poller());
	}

	Stream *get_stream(void) {
		return (this->stream);
	}

	//
	// Methods that allow one to read request/response fields
	//

	const char *get_header(const char *key) {
		const char *result;

		result = evhttp_find_header(this->headers, key);
		if (result == NULL)
			return (NULL);

		return (result);
	}

	unsigned get_status(void) {
		return (this->parser->status_code);
	}

	const char *get_reason(void) {
		return (NULL);
	}

	virtual ~Http(void) {
		if (this->body)
			delete (this->body);
		if (this->buffer)
			delete (this->buffer);
		// handler: nothing to do
		if (this->headers) {
			evhttp_clear_headers(this->headers);
			free(this->headers);
		}
		if (this->key)
			delete (this->key);
		if (this->parser)
			free(this->parser);
		// prev: nothing to do
		if (this->reason)
			delete (this->reason);
		if (this->settings)
			free(this->settings);
		if (this->stream)
			this->stream->close();
		if (this->url)
			delete (this->url);
		if (this->value)
			delete (this->body);
	}
};

#endif  /* __cplusplus */
#endif  /* NEUBOT_HTTP_HH */

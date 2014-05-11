/* libneubot/http.cpp */

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

#include "http.h"

// Definitions related to headers parsing:
#define CALLER_NOTHING 0
#define CALLER_KEY 1
#define CALLER_VALUE 2
#define CALLER_EOH 3

NeubotHttpHandler::~NeubotHttpHandler(void)
{
	// nothing
}

int
NeubotHttp::on_message_begin(http_parser *parser)
{
	NeubotHttp *self = (NeubotHttp *) parser->data;

	if (self->key->clear() != 0 || self->value->clear() != 0 ||
	    self->reason->clear() != 0 || self->url->clear() != 0) {
		neubot_warn("NeubotHttp: clear failed");
		return (-1);
	}

	self->prev = CALLER_NOTHING;
	self->handler->on_message_begin();
	return (0);
}

int
NeubotHttp::on_url(http_parser *parser, const char *data, size_t len)
{
	NeubotHttp *self = (NeubotHttp *) parser->data;
	return (self->url->append(data, len));
}

int
NeubotHttp::on_status(http_parser *parser, const char *data, size_t len)
{
	NeubotHttp *self = (NeubotHttp *) parser->data;
	return (self->reason->append(data, len));
}

int
NeubotHttp::headers_fsm(unsigned caller, const char *data, size_t len)
{
	NeubotStringBuffer *buff;
	int result, needsave;

	neubot_info("http:headers_fsm - bytes=%ld, caller=%u", len, caller);

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

	} else if (this->prev == CALLER_VALUE && caller == CALLER_VALUE) {
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

		const char *key = this->key->get_string();
		if (key == NULL) {
			neubot_warn("http:headers_fsm - cannot read key");
			return (-1);
		}

		const char *value = this->value->get_string();
		if (value == NULL) {
			neubot_warn("http:headers_fsm - cannot read value");
			return (-1);
		}

		neubot_info("< %s: %s", key, value);
		this->handler->on_header(key, value);

		neubot_info("http:headers_fsm - needsave leave");

		if (this->key->clear() != 0 || this->value->clear() != 0) {
			neubot_warn("http - cannot clear headers");
			return (-1);
		}
	}

	if (buff != NULL && buff->append(data, len) != 0) {
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

int
NeubotHttp::on_header_field(http_parser *parser, const char *data, size_t len)
{
	NeubotHttp *self = (NeubotHttp *) parser->data;
	return (self->headers_fsm(CALLER_KEY, data, len));
}

int
NeubotHttp::on_header_value(http_parser *parser, const char *data, size_t len)
{
	NeubotHttp *self = (NeubotHttp *) parser->data;
	return (self->headers_fsm(CALLER_VALUE, data, len));
}

int
NeubotHttp::on_headers_complete(http_parser *parser)
{
	NeubotHttp *self = (NeubotHttp *) parser->data;

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

int
NeubotHttp::on_body(http_parser *parser, const char *data, size_t length)
{
	NeubotHttp *self = (NeubotHttp *) parser->data;
	NeubotBytes bytes(data, length);

	self->handler->on_body(&bytes);
	return (0);
}

int
NeubotHttp::on_message_complete(http_parser *parser)
{
	NeubotHttp *self = (NeubotHttp *) parser->data;
	self->handler->on_message_complete();
	return (0);
}

NeubotHttp(HttpHandler *handler, long long fileno, unsigned isclient)
    : NeubotProtocol()
{
	//
	// Start clear
	//

	this->buffer = NULL;
	this->connection = NULL;
	this->handler = NULL;
	this->headers = NULL;
	this->key = NULL;
	this->parser = NULL;
	this->prev = CALLER_NOTHING;
	this->reason = NULL;
	this->settings = NULL;
	this->url = NULL;
	this->value = NULL;

	neubot_info("construct - enter");

	//
	// Sanity
	//

	if (handler == NULL) {
		neubot_warn("construct - invalid handler");
		throw (new (std::bad_alloc()));
	}

	NeubotPoller *poller = handler->get_poller();
	if (poller == NULL) {
		neubot_warn("construct - invalid poller");
		throw (new (std::bad_alloc()));
	}

	// fileno: checked by connection

	//
	// Alloc
	//

	this->buffer = new NeubotStringBuffer(poller);

	// connection: is constructed later

	// handler: nothing to do

	this->headers = (evkeyvalq *) calloc(1, sizeof (*this->headers));
	if (this->headers == NULL) {
		neubot_warn("construct - cannot construct headers");
		throw (new (std::bad_alloc()));
	}

	this->key = new NeubotStringBuffer(poller);

	this->parser = (http_parser *) calloc(1, sizeof (*this->parser));
	if (this->parser == NULL) {
		neubot_warn("construct - cannot construct parser");
		throw (new (std::bad_alloc()));
	}

	// prev: nothing to do

	this->reason = new NeubotStringBuffer(poller);

	this->settings = (http_parser_settings *) calloc(1,
	    sizeof (*this->settings));
	if (this->settings == NULL) {
		neubot_warn("construct - cannot construct settings");
		throw (new (std::bad_alloc()));
	}

	this->url = new NeubotStringBuffer(poller);

	this->value = new NeubotStringBuffer(poller);

	//
	// Init
	//

	this->handler = handler;

	TAILQ_INIT(this->headers);

	if (isclient)
		http_parser_init(this->parser, HTTP_RESPONSE);
	else
		http_parser_init(this->parser, HTTP_REQUEST);

	this->parser->data = this;

	this->settings->on_message_begin = this->on_message_begin;
	this->settings->on_url = this->on_url;
	this->settings->on_status = this->on_status;
	this->settings->on_header_field = this->on_header_field;
	this->settings->on_header_value = this->on_header_value;
	this->settings->on_headers_complete = this->on_headers_complete;
	this->settings->on_body = this->on_body;
	this->settings->on_message_complete = this->on_message_complete;

	//
	// Construct the connection
	//

	this->connection = NeubotConnection::attach(this, fileno);
	if (this->connection == NULL) {
		neubot_warn("construct - cannot construct connection");
		throw (new (std::bad_alloc()));
	}

	//
	// Success
	//

	neubot_info("construct - leave");
}

void
NeubotHttp::on_data(void)
{
	neubot_info("http:on_data - enter");

	int result = this->connection->read_into_(this->buffer);
	if (result != 0) {
		neubot_warn("Http - cannot read buffer");
		this->handler->on_error();
		return;
	}

	//
	// TODO: avoid making a copy
	//

	const char *data = (const char *) evbuffer_pullup(this->buffer, -1);
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

void
NeubotHttp::on_flush(void)
{
	// nothing
}

void
NeubotHttp::on_eof(void)
{
	this->handler->on_error();  // Inaccurate
}

void
NeubotHttp::on_error(void)
{
	this->handler->on_error();
}

NeubotPoller *
NeubotHttp::get_poller(void)
{
	return (this->handler->get_poller());
}

NeubotConnection *
NeubotHttp::get_connection(void)
{
	return (this->connection);
}

NeubotHttp::~NeubotHttp(void)
{
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

/*-
 * Copyright (c) 2013-2014
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

#include <new>
#include <stdlib.h>
#include <stdio.h>

#include "_connect.h"
#include "neubot_http.hh"
#include "neubot.h"
#include "log.h"

struct MyHandler : public HttpHandler {
	NeubotPoller *poller;
	Http *proto;
	Buffer *buffer;

	static MyHandler *construct(NeubotPoller *poller, long long fileno) {
		int result;

		neubot_info("MyHandler - enter");

		MyHandler *self = new (std::nothrow) MyHandler();
		if (self == NULL) {
			neubot_warn("MyHandler - cannot attach");
			return (NULL);
		}

		self->poller = poller;

		self->proto = Http::construct(self, fileno, 1);
		if (self->proto == NULL) {
			neubot_warn("MyHandler - cannot attach");
			delete self;
			return (NULL);
		}

		self->buffer = Buffer::construct(poller);
		if (self->buffer == NULL) {
			neubot_warn("MyHandler - cannot create buffer");
			delete self;
			return (NULL);
		}

		/// This shouldn't probably be done here...

		result = self->buffer->write_string("GET / HTTP/1.1\r\n");
		if (result != 0) {
			neubot_warn("MyHandler - cannot create buffer");
			delete self;
			return (NULL);
		}

		result = self->buffer->write_string("Host: nexa.polito.it\r\n");
		if (result != 0) {
			neubot_warn("MyHandler - cannot create buffer");
			delete self;
			return (NULL);
		}

		result = self->buffer->write_string("\r\n");
		if (result != 0) {
			neubot_warn("MyHandler - cannot create buffer");
			delete self;
			return (NULL);
		}

		Stream *stream = self->proto->get_stream();
		if (stream == NULL) {
			neubot_warn("MyHandler - cannot create buffer");
			delete self;
			return (NULL);
		}

		result = stream->write_buffer(self->buffer);
		if (result != 0) {
			neubot_warn("MyHandler - cannot create buffer");
			delete self;
			return (NULL);
		}

		neubot_info("MyHandler - ok");

		return (self);
	};

        virtual void on_message_begin(void) {
	};

        virtual void on_headers_complete(void) {
		const char *value;

		value = this->proto->get_header("Server");
		if (value == NULL)
			return;

		printf("%s", value);
	};

        virtual void on_body(Buffer *buffer) {
		printf("-~- %ld\n", buffer->get_length());
		printf("%s\n", buffer->read_string());
	};

        virtual void on_message_complete(void) {
		delete (this);
		NeubotPoller_break_loop(this->get_poller());
	};

        virtual void on_error(void) {
		NeubotPoller_break_loop(this->get_poller());
	};

        virtual NeubotPoller *get_poller(void) {
		return (this->poller);
	};

	virtual ~MyHandler(void) {
		// FIXME: leak
	};
};

static void
http_connected(NeubotPoller *poller, long long fileno, double rtt, void *opaque)
{
	MyHandler *handler;

	neubot_info("http_connected - enter");

	// Suppress warning concerning unused variable
	(void) rtt;
	(void) opaque;

	if (fileno == -1) {
		neubot_warn("http_connected - not connected");
		NeubotPoller_break_loop(poller);
		return;
	}

	handler = MyHandler::construct(poller, fileno);
	if (handler == NULL) {
		neubot_warn("http_connected - cannot attach");
		NeubotPoller_break_loop(poller);
		return;
	}

	neubot_info("http_connected - leave");
}

int
main(void)
{
	neubot_info("test_http - allocating poller...");

	NeubotPoller *poller = NeubotPoller_construct();
	if (poller == NULL)
		exit(EXIT_FAILURE);

	neubot_info("test_http - connecting...");

	int result = neubot_connect(poller, "PF_INET", "130.192.16.172",
	    "80", 7.0, http_connected, NULL);
	if (result != 0)
		exit(EXIT_FAILURE);

	neubot_info("test_http - poller loop...");
	NeubotPoller_loop(poller);

	neubot_info("test_http - exit");
	exit(EXIT_SUCCESS);
}

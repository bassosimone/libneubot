/* libneubot/http.h */

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

#ifndef LIBNEUBOT_HTTP_HH
# define LIBNEUBOT_HTTP_HH
# ifdef __cplusplus

struct NeubotBytes;
struct NeubotStringBuffer;

struct NeubotHttpHandler {
	virtual void on_message_begin(void) {
		// TODO: override
	}

	virtual void on_header(const char *key, const char *value) {
		// TODO: override
	}

	virtual void on_headers_complete(void) {
		// TODO: override
	}

	virtual void on_body(NeubotBytes *chunk) {
		// TODO: override
	}

	virtual void on_message_complete(void) {
		// TODO: override
	}

	virtual void on_error(void) {
		// TODO: override
	}

	// Defined out-of-line to avoid -Wweak-vtables warning
	virtual NeubotPoller *get_poller(void);

	virtual ~NeubotHttpHandler(void) {
		// TODO: override
	}
};

struct NeubotHttp : public NeubotProtocol {

	evbuffer *buffer;
	NeubotConnection *connection;
	HttpHandler *handler;
	struct evkeyvalq *headers;
	NeubotStringBuffer *key;
	http_parser *parser;
	unsigned prev;
	NeubotStringBuffer *reason;
	http_parser_settings *settings;
	NeubotStringBuffer *url;
	NeubotStringBuffer *value;

	static int on_message_begin(http_parser *parser);
	static int on_url(http_parser *, const char *, size_t);
	static int on_status(http_parser *, const char *, size_t);

	int headers_fsm(unsigned, const char *, size_t);

	static int on_header_field(http_parser *, const char *, size_t);
	static int on_header_value(http_parser *, const char *, size_t);

	static int on_headers_complete(http_parser *parser);

	static int on_body(http_parser *parser, const char *b, size_t n) {
		NeubotBytes chunk(b, n);
		NeubotHttp *self = (NeubotHttp *) parser->data;
		this->handler->on_body(&chunk);
		return (0);
	}

	static int on_message_complete(http_parser *parser) {
		NeubotHttp *self = (NeubotHttp *) parser->data;
		this->handler->on_message_complete();
		return (0);
	}

    public:
	NeubotHttp(void);

	virtual void on_data(void);
	virtual void on_flush(void);
	virtual void on_eof(void);
	virtual void on_error(void);
	virtual NeubotPoller *get_poller(void);

	NeubotConnection *get_connection(void);

	virtual ~NeubotHttp(void);
};

# endif  /* __cplusplus */
#endif  /* LIBNEUBOT_HTTP_HH */

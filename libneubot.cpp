/* libneubot/libneubot.cpp */

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

//
// Libneubot C wrappers for C++
//
// Ideally this file should be as much autogenerated as possible
//

#include <new>
#include <stdlib.h>

#include <event2/event.h>

#include "connection.h"
#include "log.h"
#include "neubot.h"
#include "protocol.h"
#include "pollable.hh"
#include "stringvector.h"

//
// Pollable
//

struct NeubotPollableWrapper : public NeubotPollable {

	neubot_slot_vo on_handle_error;
	neubot_slot_vo on_handle_read;
	neubot_slot_vo on_handle_write;
	void *opaque;

	NeubotPollableWrapper(NeubotPoller *poller, neubot_slot_vo on_read,
	    neubot_slot_vo on_write, neubot_slot_vo on_error,
	    void *opaque) : NeubotPollable(poller) {
		neubot_info("NeubotPollable::construct");
		this->on_handle_error = on_error;
		this->on_handle_read = on_read;
		this->on_handle_write = on_write;
		this->opaque = opaque;
	};

	virtual void handle_read(void) {
		neubot_info("NeubotPollable::handle_read");
		this->on_handle_read(this->opaque);
	};

	virtual void handle_write(void) {
		neubot_info("NeubotPollable::handle_write");
		this->on_handle_write(this->opaque);
	};

	virtual void handle_error(void) {
		neubot_info("NeubotPollable::handle_error");
		this->on_handle_error(this->opaque);
	};

	virtual ~NeubotPollableWrapper(void) {
		neubot_info("NeubotPollable::~NeubotPollable");
	};
};

NeubotPollable *
NeubotPollable_construct(NeubotPoller *poller, neubot_slot_vo handle_read,
    neubot_slot_vo handle_write, neubot_slot_vo handle_error,
    void *opaque)
{
	if (poller == NULL)
		abort();

	try {
		return (new NeubotPollableWrapper(poller, handle_read, handle_write,
		    handle_error, opaque));
	} catch (...) {
		return (NULL);
	}
}

int
NeubotPollable_attach(NeubotPollable *self, long long fileno)
{
	if (self == NULL)
		abort();

	return (self->attach(fileno));
}

void
NeubotPollable_detach(NeubotPollable *self)
{
	if (self == NULL)
		abort();

	self->detach();
}

long long
NeubotPollable_get_fileno(NeubotPollable *self)
{
	if (self == NULL)
		abort();

	return (self->get_fileno());
}

int
NeubotPollable_set_readable(NeubotPollable *self)
{
	if (self == NULL)
		abort();

	return (self->set_readable());
}

int
NeubotPollable_unset_readable(NeubotPollable *self)
{
	if (self == NULL)
		abort();

	return (self->unset_readable());
}

int
NeubotPollable_set_writable(NeubotPollable *self)
{
	if (self == NULL)
		abort();

	return (self->set_writable());
}

int
NeubotPollable_unset_writable(NeubotPollable *self)
{
	if (self == NULL)
		abort();

	return (self->unset_writable());
}

void
NeubotPollable_set_timeout(NeubotPollable *self, double timeout)
{
	if (self == NULL)
		abort();

	self->set_timeout(timeout);
}

void
NeubotPollable_clear_timeout(NeubotPollable *self)
{
	if (self == NULL)
		abort();

	self->clear_timeout();
}

void
NeubotPollable_close(NeubotPollable *self)
{
	if (self == NULL)
		abort();

	delete (self);
}

//
// Protocol
//

struct NeubotProtocol : public Neubot::Protocol {
	NeubotPoller *poller;
	neubot_slot_vo fn_connect;
	neubot_slot_vo fn_ssl;
	neubot_slot_vo fn_data;
	neubot_slot_vo fn_flush;
	neubot_slot_vo fn_eof;
	neubot_slot_vo fn_error;
	void *opaque;

	NeubotProtocol(NeubotPoller *p, neubot_slot_vo slot_connect,
	    neubot_slot_vo slot_ssl, neubot_slot_vo slot_data,
	    neubot_slot_vo slot_flush, neubot_slot_vo slot_eof,
	    neubot_slot_vo slot_error, void *o) {
		this->poller = p;
		this->fn_connect = slot_connect;
		this->fn_ssl = slot_ssl;
		this->fn_data = slot_data;
		this->fn_flush = slot_flush;
		this->fn_eof = slot_eof;
		this->fn_error = slot_error;
		this->opaque = o;
	}

	virtual void on_connect(void) {
		if (this->fn_connect != NULL)
			this->fn_connect(this->opaque);
	}

	virtual void on_ssl(void) {
		if (this->fn_ssl != NULL)
			this->fn_ssl(this->opaque);
	}

	virtual void on_data(void) {
		if (this->fn_data != NULL)
			this->fn_data(this->opaque);
	}

	virtual void on_flush(void) {
		if (this->fn_flush != NULL)
			this->fn_flush(this->opaque);
	}

	virtual void on_eof(void) {
		if (this->fn_eof != NULL)
			this->fn_eof(this->opaque);
	}

	virtual void on_error(void) {
		if (this->fn_error != NULL)
			this->fn_error(this->opaque);
	}

	// Defined out-of-line to avoid -Wweak-vtables warning
	virtual NeubotPoller *get_poller(void);
};

// Defined here to avoid -Wweak-vtables warning
NeubotPoller *
NeubotProtocol::get_poller(void)
{
	return (this->poller);
}

NeubotProtocol *
NeubotProtocol_construct(NeubotPoller *p, neubot_slot_vo slot_connect,
    neubot_slot_vo slot_ssl, neubot_slot_vo slot_data,
    neubot_slot_vo slot_flush, neubot_slot_vo slot_eof,
    neubot_slot_vo slot_error, void *o)
{
	if (p == NULL)
		abort();

	return (new (std::nothrow) NeubotProtocol(p, slot_connect, slot_ssl,
	    slot_data, slot_flush, slot_eof, slot_error, o));
}

NeubotPoller *
NeubotProtocol_get_poller(NeubotProtocol *self)
{
	if (self == NULL)
		abort();

	return (self->get_poller());
}

void
NeubotProtocol_destruct(NeubotProtocol *self)
{
	if (self == NULL)
		abort();

	delete (self);
}

//
// Connection
//

struct NeubotConnection : public Neubot::Connection {
	// nothing
};

NeubotConnection *
NeubotConnection_attach(NeubotProtocol *proto, long long filenum)
{
	return (static_cast<NeubotConnection *>(
	    NeubotConnection::attach(proto, filenum)));
}

NeubotConnection *
NeubotConnection_connect(NeubotProtocol *proto, const char *family,
    const char *address, const char *port)
{
	return (static_cast<NeubotConnection *>(
	    NeubotConnection::connect(proto, family, address, port)));
}

NeubotConnection *
NeubotConnection_connect_hostname(NeubotProtocol *proto, const char *family,
    const char *address, const char *port)
{
	return (static_cast<NeubotConnection *>(
	    NeubotConnection::connect_hostname(proto, family, address, port)));
}

NeubotProtocol *
NeubotConnection_get_protocol(NeubotConnection *self)
{
	if (self == NULL)
		abort();

	return (static_cast<NeubotProtocol *>(self->get_protocol()));
}

int
NeubotConnection_set_timeout(NeubotConnection *self, double timeo)
{
	if (self == NULL)
		abort();

	return (self->set_timeout(timeo));
}

int
NeubotConnection_clear_timeout(NeubotConnection *self)
{
	if (self == NULL)
		abort();

	return (self->clear_timeout());
}

int
NeubotConnection_start_tls(NeubotConnection *self, unsigned server_side)
{
	if (self == NULL)
		abort();

	return (self->start_tls(server_side));
}

int
NeubotConnection_read(NeubotConnection *self, char *base, size_t count)
{
	if (self == NULL)
		abort();

	return (self->read(base, count));
}

int
NeubotConnection_readline(NeubotConnection *self, char *base, size_t count)
{
	if (self == NULL)
		abort();

	return (self->readline(base, count));
}

int
NeubotConnection_readn(NeubotConnection *self, char *base, size_t count)
{
	if (self == NULL)
		abort();

	return (self->readn(base, count));
}

int
NeubotConnection_discardn(NeubotConnection *self, size_t count)
{
	if (self == NULL)
		abort();

	return (self->discardn(count));
}

int
NeubotConnection_write(NeubotConnection *self, const char *base, size_t count)
{
	if (self == NULL)
		abort();

	return (self->write(base, count));
}

int
NeubotConnection_puts(NeubotConnection *self, const char *str)
{
	if (self == NULL)
		abort();

	return (self->puts(str));
}

int
NeubotConnection_read_into_(NeubotConnection *self, evbuffer *dest)
{
	if (self == NULL)
		abort();

	return (self->read_into_(dest));
}

int
NeubotConnection_write_from_(NeubotConnection *self, evbuffer *source)
{
	if (self == NULL)
		abort();

	return (self->write_from_(source));
}

void
NeubotConnection_close(NeubotConnection *self)
{
	if (self == NULL)
		abort();

	self->close();
}

//
// StringVector
//

struct NeubotStringVector : public Neubot::StringVector {
	// nothing
};

NeubotStringVector *
NeubotStringVector_construct(NeubotPoller *poller, size_t count)
{
	return (static_cast<NeubotStringVector *>(
	    NeubotStringVector::construct(poller, count)));
}

int
NeubotStringVector_append(NeubotStringVector *self, const char *str)
{
	if (self == NULL)
		abort();

	return (self->append(str));
}

NeubotPoller *
NeubotStringVector_get_poller(NeubotStringVector *self)
{
	if (self == NULL)
		abort();

	return (self->get_poller());
}

const char *
NeubotStringVector_get_next(NeubotStringVector *self)
{
	if (self == NULL)
		abort();

	return (self->get_next());
}

void
NeubotStringVector_destruct(NeubotStringVector *self)
{
	if (self == NULL)
		abort();

	delete (self);
}

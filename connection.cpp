/* libneubot/connection.cpp */

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

#include <arpa/inet.h>

#include <limits.h>
#include <new>
#include <stdlib.h>
#include <string.h>

#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/dns.h>
#include <event2/event.h>

#include "ll2sock.h"
#include "log.h"
#include "neubot.h"
#include "poller.h"
#include "protocol.h"
#include "stringvector.h"
#include "utils.h"

#include "connection.h"

Neubot::Connection::Connection(void)
{
	this->filedesc = NEUBOT_SOCKET_INVALID;
	this->bev = NULL;
	this->protocol = NULL;
	this->readbuf = NULL;
	this->closing = 0;
	this->connecting = 0;
	this->reading = 0;
	this->address = NULL;
	this->port = NULL;
	this->addrlist = NULL;
	this->_family = "PF_UNSPEC";
}

Neubot::Connection::~Connection(void)
{
	if (this->filedesc != NEUBOT_SOCKET_INVALID)
		(void) evutil_closesocket((evutil_socket_t) this->filedesc);

	if (this->bev != NULL)
		bufferevent_free(this->bev);

	// protocol: should already be dead

	if (this->readbuf != NULL)
		evbuffer_free(this->readbuf);

	// closing: nothing to be done
	// connecting: nothing to be done
	// reading: nothing to be done

	if (this->address != NULL)
		free(this->address);
	if (this->port != NULL)
		free(this->port);
	if (this->addrlist != NULL)
		delete (this->addrlist);

	// _family: nothing to be done
}

void
Neubot::Connection::handle_read(bufferevent *bev, void *opaque)
{
	Connection *self = (Connection *) opaque;
	int result;

	(void) bev;  // Suppress warning about unused variable

	result = bufferevent_read_buffer(self->bev, self->readbuf);
	if (result != 0) {
		self->protocol->on_error();
		return;
	}

	self->reading = 1;
	self->protocol->on_data();
	self->reading = 0;

	if (self->closing)
		delete (self);
}

void
Neubot::Connection::handle_write(bufferevent *bev, void *opaque)
{
	Connection *self = (Connection *) opaque;
	(void) bev;  // Suppress warning about unused variable
	self->protocol->on_flush();
}

void
Neubot::Connection::handle_event(bufferevent *bev, short what, void *opaque)
{
	Connection *self = (Connection *) opaque;

	(void) bev;  // Suppress warning about unused variable

	if (self->connecting && self->closing) {
		delete (self);
		return;
	}

	if (what & BEV_EVENT_CONNECTED) {
		self->connecting = 0;
		int result = bufferevent_enable(self->bev, EV_READ);
		if (result != 0) {
			self->protocol->on_error();
			return;
		}
		self->protocol->on_connect();
		return;
	}

	if (what & BEV_EVENT_EOF) {
		self->protocol->on_eof();
		return;
	}

	if (self->connecting) {
		neubot_info("connection::handle_event - try connect next");
		self->connect_next();
		return;
	}

	// TODO: also handle the timeout

	self->protocol->on_error();
}

Neubot::Connection *
Neubot::Connection::attach(Neubot::Protocol *proto, long long filenum)
{
	event_base *evbase;
	NeubotPoller *poller;
	Neubot::Connection *self;
	int result;

	if (proto == NULL)
		abort();
	poller = proto->get_poller();
	if (poller == NULL)
		abort();
	evbase = NeubotPoller_event_base_(poller);
	if (evbase == NULL)
		abort();

	if (!neubot_socket_valid(filenum))
		return (NULL);

	self = new (std::nothrow) Neubot::Connection();
	if (self == NULL)
		return (NULL);

	// filedesc: only set on success

	self->bev = bufferevent_socket_new(evbase, (evutil_socket_t)filenum,
	    BEV_OPT_DEFER_CALLBACKS);
	if (self->bev == NULL) {
		delete self;
		return (NULL);
	}

	self->protocol = proto;

	self->readbuf = evbuffer_new();
	if (self->readbuf == NULL) {
		delete self;
		return (NULL);
	}

	// closing: nothing to be done
	// connecting: nothing to be done
	// reading: nothing to be done

	self->address = strdup("0.0.0.0");
	if (self->address == NULL) {
		delete self;
		return (NULL);
	}

	self->port = strdup("0");
	if (self->address == NULL) {
		delete self;
		return (NULL);
	}

	self->addrlist = Neubot::StringVector::construct(poller, 16);
	if (self->addrlist == NULL) {
		delete self;
		return (NULL);
	}

	// _family: nothing to be done

	bufferevent_setcb(self->bev, self->handle_read, self->handle_write,
	    self->handle_event, self);

	result = bufferevent_enable(self->bev, EV_READ);
	if (result != 0) {
		delete self;
		return (NULL);
	}

	// Only own the filedesc when we know there were no errors
	self->filedesc = filenum;
	return (self);
}

Neubot::Connection *
Neubot::Connection::connect(Neubot::Protocol *proto, const char *family,
    const char *address, const char *port)
{
	event_base *evbase;
	NeubotPoller *poller;
	int result;
	Neubot::Connection *self;
	struct sockaddr_storage storage;
	socklen_t total;

	if (proto == NULL || family == NULL || address == NULL || port == NULL)
		abort();
	poller = proto->get_poller();
	if (poller == NULL)
		abort();
	evbase = NeubotPoller_event_base_(poller);
	if (evbase == NULL)
		abort();

	result = neubot_storage_init(&storage, &total, family, address, port);
	if (result != 0)
		return (NULL);

	self = new (std::nothrow) Neubot::Connection();
	if (self == NULL)
		return (NULL);

	self->filedesc = neubot_socket_create(storage.ss_family,
	    SOCK_STREAM, 0);
	if (self->filedesc == NEUBOT_SOCKET_INVALID) {
		delete self;
		return (NULL);
	}

	self->bev = bufferevent_socket_new(evbase,
	    (evutil_socket_t)self->filedesc, BEV_OPT_DEFER_CALLBACKS);
	if (self->bev == NULL) {
		delete self;
		return (NULL);
	}

	self->protocol = proto;

	self->readbuf = evbuffer_new();
	if (self->readbuf == NULL) {
		delete self;
		return (NULL);
	}

	// closing: nothing to be done

	self->connecting = 1;

	// reading: nothing to be done

	self->address = strdup(address);
	if (self->address == NULL) {
		delete self;
		return (NULL);
	}

	self->port = strdup(port);
	if (self->address == NULL) {
		delete self;
		return (NULL);
	}

	self->addrlist = Neubot::StringVector::construct(poller, 16);
	if (self->addrlist == NULL) {
		delete self;
		return (NULL);
	}

	// _family: nothing to be done

	bufferevent_setcb(self->bev, self->handle_read, self->handle_write,
	    self->handle_event, self);

	// Note: cannot enable EV_READ until the connection is made

	result = bufferevent_socket_connect(self->bev, (struct sockaddr *)
	    &storage, (int) total);
	if (result != 0) {
		delete self;
		return (NULL);
	}

	return (self);
}

void
Neubot::Connection::connect_next(void)
{
	const char *address;
	int error;
	struct sockaddr_storage storage;
	socklen_t total;

	neubot_info("connect_next - enter");

	for (;;) {
		address = this->addrlist->get_next();
		if (address == NULL) {
			neubot_warn("connect_next - no more available addrs");
			break;
		}

		error = neubot_storage_init(&storage, &total, this->_family,
		    address, this->port);
		if (error != 0)
			continue;

		this->filedesc = neubot_socket_create(storage.ss_family,
		    SOCK_STREAM, 0);
		if (this->filedesc == NEUBOT_SOCKET_INVALID)
			continue;

		error = bufferevent_setfd(this->bev, (evutil_socket_t)
		    this->filedesc);
		if (error != 0) {
			(void) evutil_closesocket(this->filedesc);
			this->filedesc = NEUBOT_SOCKET_INVALID;
			continue;
		}

		// Note: cannot enable EV_READ until the connection is made

		error = bufferevent_socket_connect(this->bev, (struct
		    sockaddr *) &storage, (int) total);
		if (error != 0) {
			(void) evutil_closesocket(this->filedesc);
			this->filedesc = NEUBOT_SOCKET_INVALID;
			error = bufferevent_setfd(this->bev,
			    NEUBOT_SOCKET_INVALID);
			if (error != 0) {
				neubot_warn("connect_next - internal error");
				break;
			}
			continue;
		}

		neubot_info("connect_next - ok");
		return;
	}

	this->connecting = 0;
	this->protocol->on_error();
}

void
Neubot::Connection::handle_resolve(int result, char type, int count,
    int ttl, void *addresses, void *opaque)
{
	Neubot::Connection *self = (Neubot::Connection *) opaque;
	const char *p;
	int error, family, size;
	char string[128];

	(void) ttl;

	neubot_info("handle_resolve - enter");

	if (!self->connecting)
		abort();

	if (self->closing) {
		delete (self);
		return;
	}

	if (result != DNS_ERR_NONE)
		goto finally;

	switch (type) {
	case DNS_IPv4_A:
		neubot_info("handle_resolve - IPv4");
		family = AF_INET;
		self->_family = "PF_INET";
		size = 4;
		break;
	case DNS_IPv6_AAAA:
		neubot_info("handle_resolve - IPv6");
		family = AF_INET6;
		self->_family = "PF_INET6";
		size = 16;
		break;
	default:
		abort();
	}

	while (--count >= 0) {
		if (count > INT_MAX / size) {
			continue;
		}
		// Note: address already in network byte order
		p = inet_ntop(family, (char *)addresses + count * size,
		    string, sizeof (string));
		if (p == NULL) {
			neubot_warn("handle_resolve - inet_ntop() failed");
			continue;
		}
		neubot_info("handle_resolve - address %s", p);
		error = self->addrlist->append(string);
		if (error != 0) {
			neubot_warn("handle_resolve - cannot append");
			continue;
		}
	}

    finally:
	self->connect_next();
}

void
Neubot::Connection::resolve(void *opaque)
{
	Neubot::Connection *self = (Neubot::Connection *) opaque;

	if (!self->connecting)
		abort();

	if (self->closing) {
		delete (self);
		return;
	}

	NeubotPoller *poller = self->protocol->get_poller();
	if (poller == NULL)
		abort();

	evdns_base *dns_base = NeubotPoller_evdns_base_(poller);
	if (dns_base == NULL)
		abort();

	evdns_request *request = evdns_base_resolve_ipv4(dns_base,
	    self->address, DNS_QUERY_NO_SEARCH, self->handle_resolve, self);
	if (request == NULL) {
		self->connecting = 0;
		self->protocol->on_error();
		return;
	}
}

Neubot::Connection *
Neubot::Connection::connect_hostname(Neubot::Protocol *proto,
    const char *family, const char *address, const char *port)
{
	event_base *evbase;
	NeubotPoller *poller;
	int result;
	Neubot::Connection *self;

	if (proto == NULL || family == NULL || address == NULL || port == NULL)
		abort();
	poller = proto->get_poller();
	if (poller == NULL)
		abort();
	evbase = NeubotPoller_event_base_(poller);
	if (evbase == NULL)
		abort();

	self = new (std::nothrow) Neubot::Connection();
	if (self == NULL)
		return (NULL);

	// filedesc: nothing to be done

	self->bev = bufferevent_socket_new(evbase,
	    (evutil_socket_t) NEUBOT_SOCKET_INVALID,
	    BEV_OPT_DEFER_CALLBACKS);
	if (self->bev == NULL) {
		delete self;
		return (NULL);
	}

	self->protocol = proto;

	self->readbuf = evbuffer_new();
	if (self->readbuf == NULL) {
		delete self;
		return (NULL);
	}

	// closing: nothing to be done

	self->connecting = 1;

	// reading: nothing to be done

	self->address = strdup(address);
	if (self->address == NULL) {
		delete self;
		return (NULL);
	}

	self->port = strdup(port);
	if (self->address == NULL) {
		delete self;
		return (NULL);
	}

	self->addrlist = Neubot::StringVector::construct(poller, 16);
	if (self->addrlist == NULL) {
		delete self;
		return (NULL);
	}

	// _family: nothing to be done

	bufferevent_setcb(self->bev, self->handle_read, self->handle_write,
	    self->handle_event, self);

	// Note: cannot enable EV_READ until the connection is made

	result = NeubotPoller_sched(poller, 0.0, self->resolve, self);
	if (result != 0) {
		delete self;
		return (NULL);
	}

	return (self);
}

Neubot::Protocol *
Neubot::Connection::get_protocol(void)
{
	return (this->protocol);
}

int
Neubot::Connection::set_timeout(double timeout)
{
	struct timeval tv, *tvp;
	tvp = neubot_timeval_init(&tv, timeout);
	return (bufferevent_set_timeouts(this->bev, tvp, tvp));
}

int
Neubot::Connection::clear_timeout(void)
{
	return (this->set_timeout(-1));
}

int
Neubot::Connection::start_tls(unsigned server_side)
{
	(void) server_side;

	return (-1);  // TODO: implement
}

int
Neubot::Connection::read(char *base, size_t count)
{
	if (base == NULL || count == 0 || count > INT_MAX)
		return (-1);

	return (evbuffer_remove(this->readbuf, base, count));
}

int
Neubot::Connection::readline(char *base, size_t count)
{
	size_t eol_length = 0;
	evbuffer_ptr result = evbuffer_search_eol(this->readbuf,
	    NULL, &eol_length, EVBUFFER_EOL_CRLF);
	if (result.pos < 0) {
		if (evbuffer_get_length(this->readbuf) > count)
			return (-1);  /* line too long */
		return (0);
	}

	if ((size_t) result.pos > SSIZE_MAX - eol_length)
		return (-1);
	result.pos += eol_length;

	if ((size_t) result.pos > count)
		return (-1);  /* line too long */

	int llen = this->read(base, (size_t) result.pos);
	if (llen < 0)
		return (-1);

	int error = evbuffer_drain(this->readbuf, eol_length);
	if (error != 0)
		return (-1);

	return (llen);
}

int
Neubot::Connection::readn(char *base, size_t count)
{
	if (base == NULL || count == 0 || count > INT_MAX)
		return (-1);
	if (evbuffer_get_length(this->readbuf) < count)
		return (0);

	return (evbuffer_remove(this->readbuf, base, count));
}

int
Neubot::Connection::discardn(size_t count)
{
	if (count == 0 || count > INT_MAX)
		return (-1);
	if (evbuffer_get_length(this->readbuf) < count)
		return (0);

	return (evbuffer_drain(this->readbuf, count));
}

int
Neubot::Connection::write(const char *base, size_t count)
{
	if (base == NULL || count == 0)
		return (-1);

	return (bufferevent_write(this->bev, base, count));
}

int
Neubot::Connection::puts(const char *str)
{
	if (str == NULL)
		return (-1);

	return (this->write(str, strlen(str)));
}

int
Neubot::Connection::read_into_(evbuffer *destbuf)
{
	if (destbuf == NULL)
		return (-1);

	return (evbuffer_add_buffer(destbuf, this->readbuf));
}

int
Neubot::Connection::write_from_(evbuffer *sourcebuf)
{
	if (sourcebuf == NULL)
		return (-1);

	return (bufferevent_write_buffer(this->bev, sourcebuf));
}

void
Neubot::Connection::close(void)
{
	this->closing = 1;
	if (this->reading != 0 || this->connecting != 0)
		return;
	delete this;
}
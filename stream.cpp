// libneubot/stream.cpp

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

//
// Connected stream socket
//

#include <sys/types.h>

#include <event2/bufferevent.h>
#include <event2/dns.h>
#include <event2/dns_compat.h>
#include <event2/event.h>

#include <limits.h>
#include <new>
#include <string.h>

#include "bucket.hh"
#include "stream.hh"
#include "strtonum.h"

extern struct event_base *EVBASE;

#define STATE_INITIAL 0
#define STATE_BUSY (1 << 0)
#define STATE_CLOSING (1 << 1)
#define STATE_CLOSED (1 << 2)


// To convert long long to sockets:
#ifdef WIN32
# define EVUTIL_SOCKET_MAX UINTPTR_MAX
#else
# define EVUTIL_SOCKET_MAX INT_MAX
#endif
#if !(LLONG_MAX >= EVUTIL_SOCKET_MAX)
# error "LLONG_MAX must be larger than EVUTIL_SOCKET_MAX"
#endif


void
Neubot::Stream::handle_read(struct bufferevent *bev, void *opaque)
{
	Neubot::Stream *self = (Neubot::Stream *) opaque;
	int result;

	result = bufferevent_read_buffer(self->bev,
	    (struct evbuffer *) self->bucket);
	if (result != 0) {
		self->handle_error();
		return;
	}

	self->state |= STATE_BUSY;
	self->handle_data(self->bucket);
	self->state &= ~STATE_BUSY;

	if ((self->state & STATE_CLOSING) != 0)
		self->cleanup();
}

void
Neubot::Stream::handle_write(struct bufferevent *bev, void *opaque)
{
	Neubot::Stream *self = (Neubot::Stream *) opaque;
	self->handle_flush();
}

void
Neubot::Stream::handle_event(struct bufferevent *bev, short what, void *opaque)
{
	Neubot::Stream *self = (Neubot::Stream *) opaque;

	if ((what & (BEV_EVENT_READING|BEV_EVENT_EOF)) != 0)
		self->handle_eof();

	self->handle_error();
}



Neubot::Stream::Stream(long long fileno)
{
	if (fileno < 0 || EVUTIL_SOCKET_MAX)
		throw new std::bad_alloc();

	this->bev = bufferevent_socket_new(EVBASE, (evutil_socket_t) fileno,
	    BEV_OPT_CLOSE_ON_FREE|BEV_OPT_DEFER_CALLBACKS);
	if (this->bev == NULL)
		throw new std::bad_alloc();

	bufferevent_setcb(this->bev, this->handle_read, this->handle_write,
	    this->handle_event, this);

	this->bucket = new Neubot::EvbufferBucket();
	this->state = STATE_INITIAL;
}

void
Neubot::Stream::start_tls(unsigned server_side, const char *certfile)
{
	this->handle_error();
}



void
Neubot::Stream::close(void)
{
	if ((this->state & STATE_CLOSING) != 0)
		return;

	this->state |= STATE_CLOSING;
	if ((this->state & STATE_BUSY) != 0)
		return;

	this->cleanup();
}

void
Neubot::Stream::cleanup(void)
{
	if ((this->state & STATE_CLOSED) != 0)
		return;
	this->state |= STATE_CLOSED;
	this->handle_cleanup();
	if (this->bev != NULL)
		bufferevent_free(this->bev);
	if (this->bucket != NULL)
		delete this->bucket;
}

Neubot::Stream::~Stream(void)
{
	this->close();
}



void
Neubot::Stream::send(const void *data, size_t count)
{
	if ((this->state & STATE_CLOSING) != 0)
		return;
	if (count > 0 && bufferevent_write(this->bev, data, count) != 0)
		this->handle_error();
}



void
Neubot::Stream::set_timeout(double timeout)
{
	// TODO: implement
}

void
Neubot::Stream::clear_timeout(void)
{
	// TODO: implement
}

/* libneubot/pollable.cpp */

/*-
 * Copyright (c) 2014
 *     Nexa Center for Internet & Society, Politecnico di Torino (DAUIN),
 *     Simone Basso <bassosimone@gmail.com>
 *     and Alessandro Quaranta <alessandro.quaranta92@gmail.com>.
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
// NeubotPollable: an object that tries to be compatible with the namesake
// object implemented by Neubot.
//
// The presence of this object should facilitate the testing of a
// modified Neubot that uses libneubot as backend.
//
// The mid-term plan is to design Python objects at higher level (e.g.,
// at the stream level) that are more compatible with C++ objects.
//

#include <limits.h>
#include <new>
#include <stdlib.h>

#include <event2/event.h>

#include "ll2sock.h"
#include "log.h"
#include "poller.h"
#include "utils.h"

#include "pollable.hh"

NeubotPollable::NeubotPollable(NeubotPoller *poller)
{
	neubot_info("pollable: NeubotPollable()");

	this->poller = poller;
	this->timeout = -1.0;
	this->evread = NULL;
	this->evwrite = NULL;
	this->fileno = NEUBOT_SOCKET_INVALID;
}

static void
dispatch(evutil_socket_t filenum, short event, void *opaque)
{
	NeubotPollable *self = (NeubotPollable *) opaque;

	(void)filenum;

	if (event & EV_TIMEOUT)
		self->handle_error();
	else if (event & EV_READ)
		self->handle_read();
	else if (event & EV_WRITE)
		self->handle_write();
	else
		self->handle_error();
}

/*
 * Note: attach() is separated from construct(), because in Neubot there
 * are cases in which we create a NeubotPollable and then we attach() and detach()
 * file descriptors to it (e.g., the Connector object does that).
 */
int
NeubotPollable::attach(long long fileno)
{
	//
	// Sanity
	//

	neubot_info("pollable: attach()");

	if (this->fileno != NEUBOT_SOCKET_INVALID) {
		neubot_warn("pollable: already attached");
		return (-1);
	}	

	/*
	 * Note: `long long` simplifies the interaction with Java and
	 * shall be wide enough to hold evutil_socket_t, which is `int`
	 * on Unix and `intptr_t` on Windows.
	 */
	if (!neubot_socket_valid(fileno)) {
		neubot_warn("pollable: invalid argument");
		return (-1);
	}

	//
	// Init
	//

	event_base *evbase = NeubotPoller_event_base_(this->poller);
	if (evbase == NULL)
		abort();

	this->evread = event_new(evbase, fileno, EV_READ | EV_PERSIST,
	    dispatch, this);
	this->evwrite = event_new(evbase, fileno, EV_WRITE | EV_PERSIST,
	    dispatch, this);

	if (this->evread == NULL || this->evwrite == NULL) {
		neubot_warn("pollable: event_new() failed");
		return (-1);
	}

	// Set the fileno to indicate success
	this->fileno = fileno;

	return (0);
}

// MUST be idempotent and MUST work with half-initialized objects
void
NeubotPollable::detach(void)
{
	neubot_info("pollable: detach()");

	// poller: continue to point to the poller

	this->timeout = -1.0;

	if (this->evread != NULL) {
		event_free(this->evread);
		this->evread = NULL;
	}
	if (this->evwrite != NULL) {
		event_free(this->evwrite);
		this->evwrite = NULL;
	}

	//
	// We don't close the file descriptor because the Neubot pollable
	// class does not close the file description as well.
	//
	this->fileno = NEUBOT_SOCKET_INVALID;
}

/*
 * This name is different from the name used by Neubot because the
 * fileno() name clashes with the fileno() macro in some headers.
 */
long long
NeubotPollable::get_fileno(void)
{
	return (this->fileno);
}

#define OPERATION_SET 1
#define OPERATION_UNSET 2

int
NeubotPollable::setunset(const char *what, unsigned opcode, event *evp)
{
	struct timeval tv, *tvp;
	int result;

	neubot_info("pollable: %s()", what);

	if (this->fileno == NEUBOT_SOCKET_INVALID) {
		neubot_warn("%s: not attached", what);
		return (-1);
	}

	switch (opcode) {
	case OPERATION_SET:
		tvp = neubot_timeval_init(&tv, this->timeout);
		result = event_add(evp, tvp);
		break;
	case OPERATION_UNSET:
		result = event_del(evp);
		break;
	default:
		abort();
	}

	if (result != 0) {
		neubot_warn("%s: event_add/event_del() failed", what);
		return (-1);
	}

	return (0);
}

int
NeubotPollable::set_readable(void)
{
	return (this->setunset("set_readable", OPERATION_SET,
	    this->evread));
}

int
NeubotPollable::set_writable(void)
{
	return (this->setunset("set_writable", OPERATION_SET,
	    this->evwrite));
}

int
NeubotPollable::unset_readable(void)
{
	return (this->setunset("unset_readable", OPERATION_UNSET,
	    this->evread));
}

int
NeubotPollable::unset_writable(void)
{
	return (this->setunset("unset_writable", OPERATION_UNSET,
	    this->evwrite));
}

void
NeubotPollable::set_timeout(double timeout)
{
	this->timeout = timeout;
}

void
NeubotPollable::clear_timeout(void)
{
	this->timeout = -1.0;
}

void
NeubotPollable::handle_read(void)
{
	// TODO: override
}

void
NeubotPollable::handle_write(void)
{
	// TODO: override
}

/*
 * This is another difference wrt Neubot: in Neubot the handle_close()
 * method is called when a pollable is closed. Here, on the contrary, we
 * have not close notification, because this is the way in which the
 * destruction works on C++ (i.e., derived classes are destroyed earlier
 * than parent classes, thus, it's not possible to notify derived classes).
 */
void
NeubotPollable::handle_error(void)
{
	// TODO: override
}

NeubotPollable::~NeubotPollable(void)
{
	neubot_info("pollable: ~NeubotPollable()");
	this->detach();
}

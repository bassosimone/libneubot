// libneubot/bucket.cpp

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
// Buckets
//

#include <event2/buffer.h>

#include <new>

#include "bucket.hh"

Neubot::EvbufferBucket::EvbufferBucket(void)
{
	this->buffer = evbuffer_new();
	if (this->buffer == NULL)
		throw new std::bad_alloc();
}

size_t
Neubot::EvbufferBucket::length(void)
{
	return (evbuffer_get_length(this->buffer));
}

void *
Neubot::EvbufferBucket::bytes(void)
{
	void *result;

	result = evbuffer_pullup(this->buffer, -1);
	if (result == NULL)
		throw new std::bad_alloc();

	return (result);
}

Neubot::EvbufferBucket::~EvbufferBucket(void)
{
	evbuffer_free(this->buffer);
}

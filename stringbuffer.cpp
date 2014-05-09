/* libneubot/stringbuffer.cpp */

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

#include <new>

#include <event2/buffer.h>

#include "stringbuffer.h"

NeubotStringBuffer::NeubotStringBuffer(void)
{
	this->buff = evbuffer_new();
	if (this->buff == NULL)
		throw (new (std::bad_alloc));
	this->frozen = 0;
}

int
NeubotStringBuffer::append(const char *b, size_t n)
{
	return (evbuffer_add(this->buff, b, n));
}

const char *
NeubotStringBuffer::get_string(void)
{
	if (!this->frozen) {
		if (evbuffer_add(this->buff, "\0", 1) != 0)
			return (NULL);
		if (evbuffer_freeze(this->buff, 0) != 0)
			return (NULL);
		this->frozen = 1;
	}
	return ((const char *) evbuffer_pullup(this->buff, -1));
}

NeubotStringBuffer::~NeubotStringBuffer(void)
{
	evbuffer_free(this->buff);
}

/* libneubot/bytes.cpp */

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

#include <stdlib.h>
#include <string.h>

#include "bytes.h"

NeubotBytes::NeubotBytes(const char *b, size_t n)
{
	this->base = b;
	this->count = n;
	this->p = NULL;
}

int
NeubotBytes::set_aside(void)
{
	if (this->p != NULL)
		return (0);

	this->p = (char *) malloc(this->count);
	if (this->p == NULL)
		return (-1);

	memcpy(this->p, this->base, this->count);
	this->base = this->p;

	return (0);
}

NeubotBytes::~NeubotBytes(void)
{
	if (this->p != NULL)
		free(this->p);
}

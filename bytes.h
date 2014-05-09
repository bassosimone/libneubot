/* libneubot/bytes.h */

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

#ifndef LIBNEUBOT_BYTES_H
# define LIBNEUBOT_BYTES_H
# ifdef __cplusplus

struct NeubotBytes {
    private:
	const char *base;
	size_t count;
	char *p;
    public:
	NeubotBytes(const char *, size_t);

	const char *get_bytes(void) {
		return (this->base);
	}

	size_t get_size(void) {
		return (this->count);
	}

	int set_aside(void);
	~NeubotBytes(void);
};

# endif  /* __cplusplus */
#endif  /* LIBNEUBOT_BYTES_HH */

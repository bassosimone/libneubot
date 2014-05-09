/* libneubot/stringbuffer.h */

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

#ifndef LIBNEUBOT_STRINGBUFFER_H
# define LIBNEUBOT_STRINGBUFFER_H
# ifdef __cplusplus

struct NeubotStringBuffer {
    private:
	evbuffer *buff;
	unsigned frozen;
    public:
	NeubotStringBuffer(void);
	int append(const char *, size_t);
	const char *get_string(void);
	~NeubotStringBuffer(void);
};

# endif  /* __cplusplus */
#endif  /* LIBNEUBOT_STRINGBUFFER_HH */

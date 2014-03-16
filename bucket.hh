// libneubot/bucket.hh

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

struct evbuffer;

namespace Neubot {

    struct BytesContainer {
        size_t count;
        char *base;
    };

    class Bucket {
        public:
            virtual Bucket(void) = 0;
            virtual BytesContainer peek(void) = 0;
            virtual void clear(void) = 0;
            virtual ~Bucket(void) = 0;
    };

    class EvbufferBucket : public Bucket {
            struct evbuffer *buffer;
        public:
#ifndef SWIG
            operator struct evbuffer *(void) {
                return this->buffer;
            };
#endif
            EvbufferBucket(void);
            BytesContainer peek(void);
            void clear(void);
            ~EvbufferBucket(void);
    };
};

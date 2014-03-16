// libneubot/stream.hh

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

struct bufferevent;

namespace Neubot {

    class EvbufferBucket;

    class Stream {

            struct bufferevent *bev;

            Neubot::EvbufferBucket *bucket;

            int state;

            static void handle_read(struct bufferevent *, void *);

            static void handle_write(struct bufferevent *, void *);

            static void handle_event(struct bufferevent *, short, void *);

            void cleanup(void);



        public:

            Stream(long long);

            void start_tls(unsigned, const char *);



            void close(void);

            virtual void handle_cleanup(void) {
                // TODO: override
            };

            virtual ~Stream(void);



            virtual void handle_data(EvbufferBucket *) {
                // TODO: override
            };

            virtual void handle_eof(void) {
                // TODO: override
            };

            virtual void handle_rst(void) {
                // TODO: override
            };

            virtual void handle_error(void) {
                // TODO: override
            };



            void send(const char *bytes, size_t count);

            virtual void handle_flush(void) {
                // TODO: override
            };



            void set_timeout(double);

            void clear_timeout(void);
    };
};

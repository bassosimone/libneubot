// libneubot/connector.hh

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
// Connect a stream socket
//

struct evdns_getaddrinfo_request;
struct evutil_addrinfo;
struct bufferevent;

#ifndef neubot_status_t
# define neubot_status_t int
#endif

namespace Neubot {

    class Connector {

            struct evdns_getaddrinfo_request *request;

            long long socket;

            struct event *evwrite;

            static void handle_getaddrinfo(int,
              struct evutil_addrinfo *, void *);

            void cleanup(void);

        public:

            Connector(const char *, const char *, const char *);

            void handle_connect(neubot_status_t, long long);

            double elapsed(void);
    };
};

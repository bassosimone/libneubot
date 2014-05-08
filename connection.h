/* libneubot/connection.h */

/*-
 * Copyright (c) 2013-2014
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

#ifndef LIBNEUBOT_CONNECTION_H
# define LIBNEUBOT_CONNECTION_H
# ifdef __cplusplus

/*-
 * 3.6.4 Neubot Library Modules
 *
 *   Connection
 *     The connection is a C/C++ module that provides an abstraction,
 *     Σ, representing a stream-like network connection. As such, it
 *     is suitable to represent, e.g., a connected TCP socket (but
 *     possibly also a UNIX-domain socket). Of course, the Neubot
 *     library shall implement and export the Σ abstraction because
 *     it is the basic building block to construct TCP-based
 *     network-performance tests.
 *
 *     The features that Σ shall implement are the following. One shall
 *     be able to establish a TCP connection by providing Σ with the
 *     protocol family, the address and the port number. Moreover, the
 *     programmer shall be able to initialize Σ with a file descriptor.
 *     Additionally, once Σ is attached to a file description, it shall
 *     be possible to establish a SSL connection. In addition to the
 *     common methods that allow to receive and send data, Σ shall
 *     also export methods that allow to read a line, to read an exact
 *     amount of bytes and to discard an exact amount of bytes.
 *
 *     All the features described above are useful to efficiently
 *     implement network-performance tests. In particular, as discussed
 *     in Section 3.6.3, the functionality that allows to discard an
 *     exact number of bytes is useful to reduce the impact on performance
 *     of data copying when a test is implemented using a high-level
 *     language.
 */

struct bufferevent;
struct evbuffer;

namespace Neubot {
	class StringVector;
	struct Protocol;

	class Connection {
		long long filedesc;
		bufferevent *bev;
		Protocol *protocol;
		evbuffer *readbuf;
		unsigned int closing;
		unsigned int connecting;
		unsigned int reading;
		char *address;
		char *port;
		StringVector *addrlist;
		char *family;
		StringVector *pflist;
		unsigned int must_resolve_ipv4;
		unsigned int must_resolve_ipv6;

		Connection(void);

		// Private destructor because destruction may be delayed
		~Connection(void);

		// Libevent callbacks
		static void handle_read(bufferevent *, void *);
		static void handle_write(bufferevent *, void *);
		static void handle_event(bufferevent *, short, void *);

		// Functions used by connect_hostname()
		void connect_next(void);
		static void handle_resolve(int, char, int, int,
		    void *, void *);
		static void resolve(void *);

	    public:
		static Connection *attach(Protocol *, long long);

		static Connection *connect(Protocol *, const char *,
		    const char *, const char *);

		static Connection *connect_hostname(Protocol *, const char *,
		    const char *, const char *);

		Protocol *get_protocol(void);

		int set_timeout(double);

		int clear_timeout(void);

		int start_tls(unsigned int);

		int read(char *, size_t);
		int readline(char *, size_t);
		int readn(char *, size_t);
		int discardn(size_t);
		int write(const char *, size_t);
		int puts(const char *);

		// Internally-used zero-copy read and write
#ifndef SWIG
		int read_into_(evbuffer *);
		int write_from_(evbuffer *);
#endif

		void close(void);
	};
}

# endif  /* __cplusplus */
#endif  /* LIBNEUBOT_CONNECTION_H */

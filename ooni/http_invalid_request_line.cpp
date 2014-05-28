/* libneubot/ooni/http_invalid_request_line.cpp */

//
// Authors: Arturo Filastò and Simone Basso
//

#include <sys/types.h>

#include <iostream>
#include <string.h>

#include <event2/buffer.h>

#include <libneubot/connection.h>
#include <libneubot/log.h>
#include <libneubot/neubot.h>
#include <libneubot/protocol.h>

//
// TODO: the connection should return different error codes for
// different kind of errors:
//
// https://github.com/TheTorProject/ooni-spec/blob/master/data-formats/df-000-base.md#error-strings
//

class NeubotTCP4Sender : public NeubotProtocol
{
	NeubotConnection *connection;
	NeubotPoller *poller;
	char *send_data;
	evbuffer *received_data;

	//
	// TODO: Add the YAML::Node and fill it, to prepare
	// the OONI-style report.
	//

    public:
	NeubotTCP4Sender(NeubotPoller *poller) : NeubotProtocol()
	{
		this->connection = NULL; 
		this->poller = poller;
		this->send_data = NULL;
		this->received_data = NULL;
	}

	int sendData(const char *address, const char *port, const char *data) 
	{
		neubot_info("sendData: %s %s %s", address, port, data);

		// TODO: cleanup stuff if needed

		this->connection = NeubotConnection::connect(this, 
		    "PF_INET", address, port);
		if (this->connection == NULL) {
			return 1;
		}

		this->send_data = strdup(data);
		if (this->data == NULL) {
			return 2;
		}

		this->received_data = evbuffer_new();
		if (this->received_data == NULL) {
			return 3;
		}

		// FIXME: here we would like to terminate the test
		// after 5.0 seconds unconditionally.
		if (this->connection->set_timeout(5.0) != 0) {
			return 4;
		}

		neubot_info("sendData: OK");
		return 0;
	}

	virtual void on_connect(void) {
		neubot_info("on_connect: called");
		this->connection->puts(this->send_data);
		this->connection->enable_read();
	}

	virtual void on_data(void) {
		neubot_info("on_connect: on_data");
		if (this->connection->read_into_(this->received_data) < 0) {
			this->on_error_(4);
		}
	}

	virtual void on_eof(void) {
		std::cout << "Connection closed\n";
	}

	void on_error_(int error = -1) {
		std::cout << "An error occurred!\n";
		std::cout << "Erno: " << error << "\n";
		this->on_error();
	}

	virtual void on_error(void) {
		if (evbuffer_get_length(this->received_data) > 0) {
			(void)evbuffer_add(this->received_data, "\0", 1);
			std::cout << evbuffer_pullup(this->received_data, -1)
			    << "\n";
		}
	}

	// XXX: It is a shame that each protocol should implement this
	// method by itself
	virtual NeubotPoller *get_poller(void) {
		return (this->poller);
	}

	~NeubotTCP4Sender(void) {
		// TODO: cleanup the stuff
	}
};

struct HTTPInvalidRequestLine : public NeubotTCP4Sender
{
	HTTPInvalidRequestLine(NeubotPoller *poller) : NeubotTCP4Sender(poller)
	{
		// nothing
	};	

	int random_invalid_method(void) {
		return this->sendData("213.138.109.232", "80", "Antanisblinda");
	}

	// static HTTPInvalidRequestLine *random_invalid_field_count(void){
	// 
	// }

	// static HTTPInvalidRequestLine *random_big_request_method(void){
	// 
	// }

	// static HTTPInvalidRequestLine *random_invalid_version_number(void){
	// 
	// }
};


int main(void)
{

	NeubotPoller *poller;
	HTTPInvalidRequestLine *http_invalid_request_line;
	neubot_info("echo - creating the poller...");

	poller = NeubotPoller_construct();
	if (poller == NULL)
		exit(EXIT_FAILURE);

	neubot_info("echo - creating the connection...");

	http_invalid_request_line = new HTTPInvalidRequestLine(poller);
	if (http_invalid_request_line->random_invalid_method() != 0)
		exit(EXIT_FAILURE);

	neubot_info("echo - poller loop...");

	NeubotPoller_loop(poller);

	neubot_info("echo - exit");

	exit(EXIT_SUCCESS);
}

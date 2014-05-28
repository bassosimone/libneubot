/* libneubot/ooni/http_invalid_request_line.cpp */

//
// Authors: Arturo Filastò and Simone Basso
//

#include <sys/types.h>

#include <iostream>
#include <string.h>

#include <event2/buffer.h>
#include <yaml-cpp/yaml.h>

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
	evbuffer *received_data;
	YAML::Node report;
	char *send_data;

	void testComplete(void)
	{
		std::string value;

		if (evbuffer_get_length(this->received_data) > 0) {
			(void)evbuffer_add(this->received_data, "\0", 1);
			this->report["received_data"] = evbuffer_pullup(
			    this->received_data, -1);
			value = YAML::Dump(this->report);
			neubot_info("Dump of the report: %s", value.c_str());
		}
	}

    public:

	// TODO: modify the protocol to store the poller
	NeubotTCP4Sender(NeubotPoller *poller) : NeubotProtocol()
	{
		this->connection = NULL; 
		this->poller = poller;
		this->report["tampering"] = false;
		this->received_data = NULL;
		this->send_data = NULL;
	}

	// XXX: It is a shame that each protocol should implement this
	// method by itself
	virtual NeubotPoller *get_poller(void) {
		return (this->poller);
	}
};

int sendData(const char *address, const char *port, const char *data) 
{
	neubot_info("sendData: %s %s %s", address, port, data);

	// XXX: how to know if the test is already pending?

	// Start over

	if (this->connection != NULL) {
		delete (this->connection);
		this->connection = NULL;
	}
	if (this->send_data != NULL) {
		free(this->send_data);
		this->send_data = NULL;
	}
	if (this->received_data != NULL) {
		evbuffer_free(this->received_data);
		this->received_data = NULL;
	}

	// Allocate and connect

	if ((this->connection = NeubotConnection::connect(this, 
	    "PF_INET", address, port)) == NULL) {
		return 1;
	}
	if ((this->send_data = strdup(data)) == NULL) {
		return 2;
	}
	if ((this->received_data = evbuffer_new()) == NULL) {
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

void
NeubotTCP4Sender::on_connect(void)
{
	neubot_info("http_invalid_request_line: on_connect()...");
	if (this->connection->puts(this->send_data) != 0) {
		neubot_warn("http_invalid_request_line: puts() failed");
		this->report["failure"] = "unknown_failure puts failed";
		this->finish();
		return;
	}
	if (this->connection->enable_read() != 0) {
		neubot_warn("http_invalid_request_line: enable_read() failed");
		this->report["failure"] = "unknown_failure libneubot error";
		this->finish();
		return;
	}
	neubot_info("http_invalid_request_line: on_connect()... ok");
}

void
NeubotTCP4Sender::on_flush(void)
{
	// TODO: try...except ?
	neubot_info("http_invalid_request_line: on_flush()...");
	if (!this->report["sent"]) {
		this->report["sent"] = YAML::Load("[]");
	}
	this->report["sent"].push_back(this->send_data);
	neubot_info("http_invalid_request_line: on_flush()... ok");
}

void
NeubotTCP4Sender::on_data(void)
{
	neubot_info("http_invalid_request_line: on_data()...");
	if (this->connection->read_into_(this->received_data) != 0) {
		neubot_warn("http_invalid_request_line: read_into_() failed");
		this->report["failure"] = "unknown_failure libneubot error";
		this->finish();
		return;
	}
	neubot_info("http_invalid_request_line: on_data()... ok");
}

void
NeubotTCP4Sender::on_eof(void)
{
	this->report["failure"] = "unknown_failure EOF";
	this->finish();
}

void
NeubotTCP4Sender::on_error(void)  // XXX: this is basically a timeout
{
	this->report["failure"] = "generic_timeout_error";
	this->finish();
}

NeubotTCP4Sender::~NeubotTCP4Sender(void)
{
	// TODO: cleanup the stuff
}

//
// HTTPInvalidRequestLine
//

struct HTTPInvalidRequestLine : public NeubotTCP4Sender
{
	HTTPInvalidRequestLine(NeubotPoller *poller) : NeubotTCP4Sender(poller)
	{
		// nothing
	};	

	int random_invalid_method(void)
	{
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

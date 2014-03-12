#!/usr/bin/env python
# Public domain, 2014 Simone Basso <bassosimone@gmail.com>.

""" Test for NeubotHttpClient """

import sys

from libneubot1 import HttpClient
from libneubot1 import Poller

class MyHttpClient(HttpClient):

    def __init__(self, poller):
        HttpClient.__init__(self, poller)
        self.count = 0

    def handle_close(self):
        sys.stderr.write("* close\n")

    def handle_connect(self):
        sys.stderr.write("* connect\n")
        self._make_request()

    def _make_request(self):
        self.writes("GET /robots.txt HTTP/1.1\r\n")
        self.writes("Host: www.neubot.org\r\n")
        self.writes("Accept: */*\r\n")
        self.writes("User-Agent: Neubot\r\n")
        self.writes("\r\n")
        self.flush()

    def handle_body(self):
        sys.stdout.write("%s" % self.body_string())
        sys.stdout.flush()

    def handle_end(self):
        sys.stderr.write("* end\n")
        self.count += 1
        if self.count > 2:
            self.close()
            return
        self._make_request()

    def handle_flush(self):
        sys.stderr.write("* flush\n")

def main():
    """ Main function """
    poller = Poller()
    client = MyHttpClient(poller)
    client.connect("PF_INET", "www.neubot.org", "80")
    poller.loop()

if __name__ == "__main__":
    main()

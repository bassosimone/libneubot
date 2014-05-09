#!/usr/bin/env python
# Public domain, 2014 Simone Basso <bassosimone@gmail.com>.

""" Test for StringBuffer """

import sys

sys.path.insert(0, "/usr/local/share/libneubot")

from libneubot import Poller
from libneubot import StringBuffer

def main():

    poller = Poller()
    stringbuffer = StringBuffer(poller)

    append = lambda sb, s: sb.append(s, len(s))

    append(stringbuffer, "yet another")
    append(stringbuffer, " random")
    append(stringbuffer, " str")

    sys.stdout.write("[%d] '%s'\n" % (stringbuffer.get_length(),
      stringbuffer.get_string()))

    result = append(stringbuffer, "foo")
    sys.stdout.write("Append when frozen: %d\n" % result)
    stringbuffer.clear()

    sys.stdout.write("[%d] '%s'\n" % (stringbuffer.get_length(),
      stringbuffer.get_string()))

    stringbuffer.destruct()

if __name__ == "__main__":
    main()

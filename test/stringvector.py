#!/usr/bin/env python
# Public domain, 2014 Simone Basso <bassosimone@gmail.com>.

""" Test for NeubotPollable """

import sys

sys.path.insert(0, "/usr/local/share/libneubot")

import libneubot

def main():
    """ Main function """
    poller = libneubot.NeubotPoller_construct()
    stringvector = libneubot.NeubotStringVector_construct(poller, 12)

    libneubot.NeubotStringVector_append(stringvector, "yet another")
    libneubot.NeubotStringVector_append(stringvector, "random")
    libneubot.NeubotStringVector_append(stringvector, "str")

    sys.stdout.write("%s %s %s\n" % (
      libneubot.NeubotStringVector_get_next(stringvector),
      libneubot.NeubotStringVector_get_next(stringvector),
      libneubot.NeubotStringVector_get_next(stringvector)))

    libneubot.NeubotStringVector_destruct(stringvector)

if __name__ == "__main__":
    main()

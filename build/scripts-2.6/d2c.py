#!/usr/bin/python

import sys
import getopt
import os
from d2c.AMITools import AMIToolsFactory
from d2c.Application import Application
from d2c.data.DAO import DAO

class Usage(Exception):
    def __init__(self, msg):
        self.msg = msg


def main(argv=None):
    if argv is None:
        argv = sys.argv
    try:
        try:
            opts, args = getopt.getopt(argv[1:], "h", ["help"])
        except getopt.error, msg:
            raise Usage(msg)
        # more code, unchanged
    except Usage, err:
        print >>sys.stderr, err.msg
        print >>sys.stderr, "for help use --help"
        return 2
    
    SQLITE_FILE = "%s/.d2c/d2c_db.sqlite" % os.path.expanduser('~') 
    
    app = Application(DAO(SQLITE_FILE), AMIToolsFactory())
    app.MainLoop()

if __name__ == "__main__":
    sys.exit(main())

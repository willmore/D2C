import sys
import os

from d2c.Application import Application
from d2c.data.DAO import DAO
from d2c.AMITools import AMIToolsFactory

import test_initor


def main(argv=sys.argv):
    
    SQLITE_FILE = "%s/.d2c_test/d2c_db.sqlite" % os.path.expanduser('~') 
    if os.path.exists(SQLITE_FILE):
        print "Deleting existing DB"
        os.unlink(SQLITE_FILE)
        
    dao = DAO(SQLITE_FILE)
    
    test_initor.init_db(dao, argv[1])

    app = Application(dao, AMIToolsFactory())
    app.MainLoop()

if __name__ == "__main__":
    sys.exit(main())

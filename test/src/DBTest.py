import sys
import os

from d2c.Application import Application
from d2c.data.DAO import DAO
from d2c.AMITools import AMIToolsFactory

import test_initor

SQLITE_FILE = "%s/.d2c_test/d2c_db.sqlite" % os.path.expanduser('~') 
if os.path.exists(SQLITE_FILE):
    print "Deleting existing DB"
    os.unlink(SQLITE_FILE)
        
dao = DAO(SQLITE_FILE)

test_initor.init_db(dao, "/home/willmore/scicloud.conf")

deployment = dao.getDeployments()[0]
deployment.state = "FOOBAR"
uploadAction = deployment.roles[0].uploadActions[0]
uploadAction.logger = True
dao.saveDeployment(deployment)

assert uploadAction.logger
assert uploadAction is deployment.roles[0].uploadActions[0]
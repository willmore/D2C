import sys
import os

from d2c.Application import Application
from AMIToolsStub import AMIToolsFactoryStub
from d2c.data.DAO import DAO
from d2c.model.Deployment import Role
from d2c.model.Deployment import Deployment
from d2c.model.AMI import AMI

def main(argv=None):
    
    DAO._SQLITE_FILE = "%s/.d2c_test/d2c_db.sqlite" % os.path.expanduser('~') 
    if os.path.exists(DAO._SQLITE_FILE):
        print "Deleting existing DB"
        os.unlink(DAO._SQLITE_FILE)
    dao = DAO()
    dao.addSourceImage("/foobar/vm.vdi")
    dao.createAmi("ami-xyz123", "/foobar/vm.vdi")
    
    dao.saveDeployment(Deployment("dummyDep", [Role("master", AMI("ami-xyz123", "/foobar/vm.vdi"), 1)]))
    
    app = Application(AMIToolsFactoryStub())
    app.MainLoop()

if __name__ == "__main__":
    sys.exit(main())

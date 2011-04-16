import sys
import os

from d2c.Application import Application
from d2c.data.DAO import DAO
from d2c.model.Role import Role
from d2c.model.Deployment import Deployment
from d2c.model.InstanceType import InstanceType
from d2c.AMITools import AMIToolsFactory

from TestConfig import TestConfig

def main(argv=None):
    
    DAO._SQLITE_FILE = "%s/.d2c_test/d2c_db.sqlite" % os.path.expanduser('~') 
    if os.path.exists(DAO._SQLITE_FILE):
        print "Deleting existing DB"
        os.unlink(DAO._SQLITE_FILE)
        
    dao = DAO()
    
    conf = TestConfig("/home/willmore/test.conf")
        
    dao.saveConfiguration(conf)
    
    
    dao.addSourceImage("/foobar/vm.vdi")
    amiId = "ami-47cefa33"
    dao.createAmi(amiId, "/foobar/vm.vdi")
    ami = dao.getAMIById(amiId)
    
    dao.saveDeployment(Deployment("dummyDep", roles=[Role("dummyDep", "loner", ami, 1, InstanceType.T1_MICRO)]))
   
    app = Application(AMIToolsFactory())
    app.MainLoop()

if __name__ == "__main__":
    sys.exit(main())

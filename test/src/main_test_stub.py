import sys
import os

from d2c.Application import Application
from AMIToolsStub import AMIToolsFactoryStub
from d2c.data.DAO import DAO
from d2c.model.Role import Role
from d2c.model.InstanceType import InstanceType
from d2c.model.Deployment import Deployment
from TestConfig import TestConfig

def main(argv=None):
    
    DAO._SQLITE_FILE = "%s/.d2c_test/main_test_stub.sqlite" % os.path.expanduser('~') 
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
    
    deployment = Deployment("dummyDep", 
                            roles=[Role("dummyDep", "loner", ami, 1, InstanceType.T1_MICRO)])
    dao.saveDeployment(deployment)
    
    app = Application(AMIToolsFactoryStub())
    app.MainLoop()

if __name__ == "__main__":
    sys.exit(main())

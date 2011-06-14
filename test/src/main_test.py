import sys
import os

from d2c.Application import Application
from d2c.data.DAO import DAO
from d2c.model.Role import Role
from d2c.model.Deployment import Deployment
from d2c.model.InstanceType import InstanceType
from d2c.AMITools import AMIToolsFactory
from d2c.EC2ConnectionFactory import EC2ConnectionFactory
from d2c.data.CredStore import CredStore

from TestConfig import TestConfig

def main(argv=None):
    
    SQLITE_FILE = "%s/.d2c_test/d2c_db.sqlite" % os.path.expanduser('~') 
    if os.path.exists(SQLITE_FILE):
        print "Deleting existing DB"
        os.unlink(SQLITE_FILE)
        
    dao = DAO(SQLITE_FILE)
    credStore = CredStore(dao)
    dao.setEC2ConnectionFactory(EC2ConnectionFactory(credStore))
    dao.setCredStore(credStore)
    
    conf = TestConfig("/home/willmore/scicloud.conf")
    
    dao.saveConfiguration(conf)
    
    
    dao.addSourceImage("/foobar/vm.vdi")
    amiId = "ami-47cefa33"
    dao.createAmi(amiId, "/foobar/vm.vdi")
    ami = dao.getAMIById(amiId)
    
    dao.saveDeployment(Deployment("dummyDep", 
                                  roles=[Role("dummyDep", "loner", ami, 1, InstanceType.T1_MICRO)]))
   
    app = Application(dao, AMIToolsFactory())
    app.MainLoop()

if __name__ == "__main__":
    sys.exit(main())

import sys
import os

from d2c.data.DAO import DAO
from d2c.model.Role import Role
from d2c.model.Deployment import Deployment
from d2c.model.Action import Action
from d2c.model.FileExistsFinishedCheck import FileExistsFinishedCheck
from d2c.EC2ConnectionFactory import EC2ConnectionFactory
from d2c.logger import StdOutLogger
from TestConfig import TestConfig

def main(argv=None):
    '''
    End-to-End integration test of deployment lifecycle.
    Uses one micro, EBS-backed instance,
    
    '''
    DAO._SQLITE_FILE = "%s/.d2c_test/deploy_test.sqlite" % os.path.expanduser('~') 
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
    ec2ConnFactory = EC2ConnectionFactory(conf.awsCred.access_key_id, 
                                          conf.awsCred.secret_access_key, 
                                          StdOutLogger())
    
    deployment = Deployment("dummyDep", ec2ConnFactory, 
                            [Role("dummyDep", "loner", ami, 1, 
                                  startActions = [Action('echo howdy > /tmp/howdy.txt')],
                                  finishedChecks = [FileExistsFinishedCheck("/tmp/howdy.txt")],
                                  dao=dao)])
    dao.saveDeployment(deployment)
    
    class Listener:
        
        def notify(self, event):
            print "Deployment state changed to: " + event.newState
    
    deployment.addAnyStateChangeListener(Listener())
        
    deployment.start()
    
    deployment.join()
    
    
if __name__ == "__main__":
    sys.exit(main())

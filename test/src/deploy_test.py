import sys
import os

from d2c.Application import Application
from AMIToolsStub import AMIToolsFactoryStub
from d2c.data.DAO import DAO
from d2c.model.Deployment import Role
from d2c.model.Deployment import Deployment
from d2c.model.AMI import AMI
from d2c.model.EC2Cred import EC2Cred
from d2c.model.Configuration import Configuration
from d2c.model.AWSCred import AWSCred
from d2c.AMITools import AMIToolsFactory
from d2c.EC2ConnectionFactory import EC2ConnectionFactory
from d2c.logger import StdOutLogger
import string

def main(argv=None):
    
    DAO._SQLITE_FILE = "%s/.d2c_test/deploy_test.sqlite" % os.path.expanduser('~') 
    if os.path.exists(DAO._SQLITE_FILE):
        print "Deleting existing DB"
        os.unlink(DAO._SQLITE_FILE)
    dao = DAO()
    
    settings = {}
    for l in open("/home/willmore/test.conf", "r"):
        (k, v) = string.split(l.strip(), "=")
        settings[k] = v
    
    print str(settings)
 
    ec2Cred = EC2Cred(settings['cert'], settings['privateKey'])
    
    awsCred = AWSCred(settings['accessKey'],
                      settings['secretKey'])
        
    conf = Configuration(ec2ToolHome='/opt/EC2_TOOLS',
                             awsUserId=settings['userid'],
                             ec2Cred=ec2Cred,
                             awsCred=awsCred)
        
    dao.saveConfiguration(conf)
    
    
    dao.addSourceImage("/foobar/vm.vdi")
    amiId = "ami-47cefa33"
    dao.createAmi(amiId, "/foobar/vm.vdi")
    ami = dao.getAMIById(amiId)
    ec2ConnFactory = EC2ConnectionFactory(settings['accessKey'], settings['secretKey'], StdOutLogger())
    deployment = Deployment("dummyDep", ec2ConnFactory, [Role("dummyDep", "loner", ami, 1)])
    dao.saveDeployment(deployment)
    
    class Listener:
        
        def notify(self, event):
            print "Deployment state changed to: " + event.newState
    
    deployment.addAnyStateChangeListener(Listener())
        
    deployment.run()

if __name__ == "__main__":
    sys.exit(main())

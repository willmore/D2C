import unittest
#import d2c.AMICreator
import string
import time
import sys

#sys.path.append("/home/willmore/workspace/cloud/src")
print sys.path
from d2c.logger import StdOutLogger
from d2c.model.EC2Cred import EC2Cred
from d2c.AMITools import AMITools
#import d2c.AMICreator as AMICreator
from d2c.aws.Launcher import Launcher
from d2c.EC2ConnectionFactory import EC2ConnectionFactory
from d2c.model.Deployment import Deployment, Role
from d2c.model.AMI import AMI
from d2c.data.DAO import DAO
import os
import boto.ec2.instance
from MicroMock import MicroMock

class DummyConnFactory:
    
    def getConnection(self):
        return DummyConn()
    
class DummyConn:
    
    def __init__(self):
        self.num = 0
    
    def get_image(self, ami):
        self.num += 1
        return DummyImg(self.num)
    
class DummyImg():
    
    def __init__(self, ami):
        self.ami = ami
    
    def run(self, min_count, max_count):
        instances = []
        
        for i in range(max_count):
            instances.append(MicroMock(id="i-%s.%d" % (self.ami, i)))
        
        return MicroMock(instances=instances)
        
class LauncherTest(unittest.TestCase):


    def setUp(self):
        
        DAO._SQLITE_FILE = "%s/.d2c_test/d2c_db.sqlite" % os.path.expanduser('~') 
        
        if os.path.exists(DAO._SQLITE_FILE):
            print "Deleting existing DB"
            os.unlink(DAO._SQLITE_FILE)
        
        self.dao = DAO()
           
        settings = {}
        for l in open("/home/willmore/test.conf", "r"):
            (k, v) = string.split(l.strip(), "=")
            settings[k] = v
        
 
        self.ec2Cred = EC2Cred(settings['cert'], settings['privateKey'])
    
        self.logger = StdOutLogger();
        
        ec2ConnFactory = DummyConnFactory()
        #ec2ConnFactory = EC2ConnectionFactory(settings['accessKey'], settings['secretKey'], StdOutLogger())
        self.launcher = Launcher(ec2ConnFactory,
                                 self.dao)

    def test_launch(self):
        
        # This AMI can run on a micro instance (free tier eligible)
        self.dao.addAMI("ami-47cefa33")
        ami = self.dao.getAMIById("ami-47cefa33")
        
        dName = "DummyDeployment"
        deployment = Deployment(dName, [Role(dName, "loner", ami, 2), Role(dName, "loner2", ami, 2)])
        self.dao.saveDeployment(deployment)
        
        self.launcher.launch(deployment)
            
        self.assertEqual(len(deployment.getInstances()), 4)    


if __name__ == '__main__':
    unittest.main()




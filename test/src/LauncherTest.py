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
        self.launcher = Launcher(EC2ConnectionFactory(settings['accessKey'], settings['secretKey'], StdOutLogger()))

    def test_launch(self):
        
        # This AMI can run on a micro instance (free tier eligible)
        self.dao.addAMI("ami-47cefa33")
        ami = self.dao.getAMIById("ami-47cefa33")
        
        deployment = Deployment("dummyDep", [Role("loner", ami, 1)])
        self.dao.saveDeployment(deployment)
        
        self.launcher.launch(deployment)

if __name__ == '__main__':
    unittest.main()




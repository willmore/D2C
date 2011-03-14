import unittest
import string

from d2c.logger import StdOutLogger
from d2c.model.Deployment import *
from d2c.model.DeploymentDescription import *
from d2c.model.AMI import AMI
from d2c.model.DeploymentManager import DeploymentManager

from d2c.model.EC2ConnectionFactory import EC2ConnectionFactory

class DeploymentTest(unittest.TestCase):


    def test_full(self):
        print 
        
        settings = {}
        for l in open("/home/willmore/test.conf", "r"):
            (k, v) = string.split(l.strip(), "=")
            settings[k] = v
    
        logger = StdOutLogger();
        
        
        ec2ConnectionFactory = EC2ConnectionFactory(settings['accessKey'], settings['secretKey'], "eu-west-1", logger)
        for x in ec2ConnectionFactory.getConnection().get_all_instances():
            print x.instances

if __name__ == '__main__':
    unittest.main()




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
from d2c.model.Action import Action
from d2c.AMITools import AMIToolsFactory
from d2c.model.DataCollector import DataCollector
from d2c.EC2ConnectionFactory import EC2ConnectionFactory
from d2c.logger import StdOutLogger
from boto.ec2.instance import Reservation
import random
import os
from MicroMock import MicroMock as mock

import string

def main(argv=None):
    
    DAO._SQLITE_FILE = "%s/.d2c_test/startAction_test.sqlite" % os.path.expanduser('~') 
    if os.path.exists(DAO._SQLITE_FILE):
        print "Deleting existing DB"
        os.unlink(DAO._SQLITE_FILE)
    dao = DAO()
    
    settings = {}
    for l in open("/home/willmore/test.conf", "r"):
        (k, v) = string.split(l.strip(), "=")
        settings[k] = v
    
    print str(settings)
 
    ec2Cred = EC2Cred(settings['ec2CredId'], settings['cert'], settings['privateKey'])
    
    awsCred = AWSCred(settings['accessKey'],
                      settings['secretKey'])
        
    conf = Configuration(ec2ToolHome='/opt/EC2_TOOLS',
                             awsUserId=settings['userid'],
                             ec2Cred=ec2Cred,
                             awsCred=awsCred)
        
    dao.saveConfiguration(conf)
    
    instance = mock(public_dns_name='ec2-46-51-132-232.eu-west-1.compute.amazonaws.com',
                    key_name='cloudeco')
    destFile = "/tmp/out.%d" % random.randint(0,1000)
    collector = DataCollector('/tmp/howdy.txt', destFile, dao)
    collector.execute(instance)
    
    assert os.path.exists(destFile)
        

if __name__ == "__main__":
    sys.exit(main())

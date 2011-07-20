import sys
import os


from d2c.data.DAO import DAO
from d2c.model.SourceImage import AMI
from d2c.model.EC2Cred import EC2Cred
from d2c.model.Configuration import Configuration
from d2c.model.AWSCred import AWSCred
from d2c.model.Action import Action
from d2c.EC2ConnectionFactory import EC2ConnectionFactory
from d2c.logger import StdOutLogger

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
    
  
    ec2ConnFactory = EC2ConnectionFactory(settings['accessKey'], settings['secretKey'], StdOutLogger())
    
    role = Role("dummyDep", "loner", AMI('dummy'), 1, reservationId='r-f3639a85',
                 startActions = [Action('echo howdy > /tmp/howdy.txt')], 
                 logger=StdOutLogger(), ec2ConnFactory=ec2ConnFactory, dao=dao)
    
    role.executeStartCommands()
    

if __name__ == "__main__":
    sys.exit(main())

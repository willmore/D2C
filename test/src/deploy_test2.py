import sys
import os

from d2c.data.DAO import DAO
from d2c.model.Role import Role
from d2c.model.Deployment import Deployment
from d2c.model.Action import Action
from d2c.model.DataCollector import DataCollector
from d2c.model.FileExistsFinishedCheck import FileExistsFinishedCheck
from d2c.EC2ConnectionFactory import EC2ConnectionFactory
from d2c.model.SourceImage import AMI
from d2c.model.SSHCred import SSHCred
from d2c.logger import StdOutLogger
from TestConfig import TestConfig
from threading import Thread
from d2c.model.InstanceType import InstanceType
import random

def main(argv=None):
    '''
    End-to-End integration test of deployment lifecycle.
    Uses one micro, EBS-backed instance,   
    '''
    
    SQLITE_FILE = "%s/.d2c_test/deploy_test.sqlite" % os.path.expanduser('~') 
    if os.path.exists(SQLITE_FILE):
        print "Deleting existing DB"
        os.unlink(SQLITE_FILE)
    dao = DAO(SQLITE_FILE)
    
    conf = TestConfig("/home/willmore/test.conf")
        
    dao.saveConfiguration(conf)
    
    ec2ConnFactory = EC2ConnectionFactory(conf.awsCred.access_key_id, 
                                          conf.awsCred.secret_access_key, 
                                          StdOutLogger())
    
    #credStore = CredStore(dao)
    
    testDir = "/tmp/d2c/%s/" % random.randint(0, 1000)
    
    sshCred = SSHCred('dirac', '/home/willmore/dirac.id_rsa')
    
    deployment = Deployment("dummyDep", 
                            ec2ConnFactory, 
                            roles=[
                                   Role("dummyDep", "loner", 
                                        ami=AMI(amiId="ami-1396a167"), 
                                        count=1, 
                                        instanceType=InstanceType.M1_LARGE,
                                        startActions=[Action(command="echo howdy > /tmp/howdy.txt", 
                                                             sshCred=sshCred)],
                                        finishedChecks=[FileExistsFinishedCheck(fileName="/tmp/howdy.txt", 
                                                                                sshCred=sshCred)],
                                        #stopActions=[Action(command="sudo service collectd stop", 
                                        #                    sshCred=sshCred)],
                                        dataCollectors=[DataCollector(source="/tmp/howdy.txt", 
                                                                      destination=testDir + "howdy.txt",
                                                                      sshCred=sshCred),
                                                        DataCollector(source="/opt/collectd/var/lib/collectd", 
                                                                      destination=testDir + "collectd_stats",
                                                                      sshCred=sshCred)])])
    
    class Listener:  
        def notify(self, event):
            print "Deployment state changed to: " + event.newState
    
    deployment.addAnyStateChangeListener(Listener())
        
    thread = Thread(target=deployment.run)
    thread.start()
    thread.join()
    
    
if __name__ == "__main__":
    sys.exit(main())

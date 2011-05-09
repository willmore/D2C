import sys

from d2c.model.Role import Role
from d2c.model.Deployment import Deployment
from d2c.model.Action import Action
from d2c.model.AsyncAction import AsyncAction
from d2c.model.UploadAction import UploadAction
from d2c.model.DataCollector import DataCollector
from d2c.model.FileExistsFinishedCheck import FileExistsFinishedCheck
from d2c.EC2ConnectionFactory import EC2ConnectionFactory
from d2c.model.AMI import AMI
from d2c.model.InstanceType import InstanceType
from d2c.model.SSHCred import SSHCred
from d2c.logger import StdOutLogger
from TestConfig import TestConfig

def main(argv=None):
    
    conf = TestConfig("/home/willmore/test.conf")
    ec2ConnFactory = EC2ConnectionFactory(conf.awsCred.access_key_id, 
                                          conf.awsCred.secret_access_key, 
                                          StdOutLogger())
        
    masterRole = "master"
    slaveRole = "slave"
    masterOutDir = "./results/master/"
    slaveOutDir = "./results/slave/"    
    sshCred = SSHCred('dirac', '/home/willmore/dirac.id_rsa')
    
    numHosts = 8
    script = "da-k-8nodes.py"
    doneFile = "/tmp/done.txt"
    startCmd = "mpirun -np %d -hostfile /tmp/d2c.context gpaw-python %s > experiment.out 2>&1; date > %s" % (numHosts, script, doneFile)

    instanceType = InstanceType("m1.xlarge", 2, 7500, 850, [])
    fixSSHCmd = "echo \"Host *\n   StrictHostKeyChecking no\" > .ssh/config"

    deployment = Deployment("da_k_8nodes", 
                            ec2ConnFactory, 
                            roles=[
                                   Role(masterRole, "master", 
                                        ami=AMI(amiId="ami-1396a167"), 
					                    count=1, 
                                        instanceType=instanceType,
					                    contextCred=sshCred,
                                        startActions=[UploadAction(source="./input/hpccinf.txt", 
                                                                   destination="/home/hpcc-user/hpccinf.txt", 
                                                                   sshCred=sshCred),
                                                      Action(command=fixSSHCmd,
							                                             sshCred=sshCred),
							                          AsyncAction(command=startCmd, 
                                                                  sshCred=sshCred)],
                                        finishedChecks=[FileExistsFinishedCheck(fileName=doneFile,
                                                                                sshCred=sshCred)],
                            
                                        dataCollectors=[DataCollector(source="/home/dirac", 
                                                                      destination=masterOutDir + "home_dir",
                                                                      sshCred=sshCred),
							DataCollector(source="/tmp",
                                                                      destination=masterOutDir + "tmp_dir",
                                                                      sshCred=sshCred),
                                                        DataCollector(source="/opt/collectd/var/lib/collectd", 
                                                                      destination=masterOutDir + "collectd_stats",
                                                                      sshCred=sshCred)]),
				    Role(slaveRole, "slave",
                                        ami=AMI(amiId="ami-1396a167"),
                                        count=1,
                                        instanceType=instanceType,
                                        contextCred=sshCred,
                                        startActions=[UploadAction(source="./input/hpccinf.txt", 
                                                                   destination="/home/hpcc-user/hpccinf.txt", 
                                                                   sshCred=sshCred)],
                                        dataCollectors=[DataCollector(source="/home/dirac",
                                                                      destination=slaveOutDir + "home_dir",
                                                                      sshCred=sshCred),
                                                        DataCollector(source="/opt/collectd/var/lib/collectd",
                                                                      destination=slaveOutDir + "collectd_stats",
                                                                      sshCred=sshCred)])

				])
    
    class Listener:  
        def notify(self, event):
            print "Deployment state changed to: " + event.newState
    
    deployment.addAnyStateChangeListener(Listener())
        
    deployment.run()
    
    
if __name__ == "__main__":
    sys.exit(main())

import sys

from d2c.model.Role import Role
from d2c.model.Deployment import Deployment
from d2c.model.AsyncAction import AsyncAction
from d2c.model.UploadAction import UploadAction
from d2c.model.DataCollector import DataCollector
from d2c.model.FileExistsFinishedCheck import FileExistsFinishedCheck
from d2c.EC2ConnectionFactory import EC2ConnectionFactory
from d2c.model.AMI import AMI
from d2c.model.InstanceType import InstanceType
from d2c.model.SSHCred import SSHCred
from d2c.logger import StdOutLogger
from d2c.model.FileConfiguration import FileConfiguration

def main(argv=None):
    
    '''
    Executes the HPCC test suite on fout m1.xlarge instances.
    Total cores = 16
    Total memory = 60GB
    HPCC N = sqrt(60000000 / 8) * .8 = 2190
    '''
    
    
    conf = FileConfiguration("/home/willmore/test.conf")
    ec2ConnFactory = EC2ConnectionFactory(conf.awsCred.access_key_id, 
                                          conf.awsCred.secret_access_key, 
                                          StdOutLogger())
        
    masterRole = "master"
    slaveRole = "slave"
    masterOutDir = "./results/master/"
    slaveOutDir = "./results/slave/"    
    sshCred = SSHCred('hpcc-user', '/home/willmore/.ssh/id_rsa_nopw')
    
    numCores = 16 
    doneFile = "/tmp/done.txt"
    startCmd = "mpirun -np %d -hostfile /tmp/d2c.context hpcc %s > experiment.out 2>&1; date > %s" % (numCores, "/home/hpcc-user/hpccinf.txt", doneFile)
    

    instanceType = InstanceType("m1.xlarge", 4, 7500, 1500, [])

    cloudHppcAMI = AMI(amiId="ami-295e685d")

    collectdSrc = "/var/lib/collectd/rrd/cloud-hpcc"

    deployment = Deployment("cloud_hpcc", 
                            ec2ConnFactory, 
                            roles=[
                                   Role(masterRole, "master", 
                                        ami=cloudHppcAMI, 
					                    count=1, 
                                        instanceType=instanceType,
					                    contextCred=sshCred,
                                        startActions=[UploadAction(source="./input/hpccinf.txt", 
                                                                   destination="/home/hpcc-user/hpccinf.txt", 
                                                                   sshCred=sshCred),
                                                      AsyncAction(command=startCmd, 
                                                                  sshCred=sshCred)],
                                        finishedChecks=[FileExistsFinishedCheck(fileName=doneFile,
                                                                                sshCred=sshCred)],
                            
                                        dataCollectors=[DataCollector(source="/home/hpcc-user", 
                                                                      destination=masterOutDir + "home_dir",
                                                                      sshCred=sshCred),
                                                        DataCollector(source=collectdSrc, 
                                                                      destination=masterOutDir + "collectd_stats",
                                                                      sshCred=sshCred)]),
				                    Role(slaveRole, "slave",
                                         ami=cloudHppcAMI,
                                         count=3,
                                         instanceType=instanceType,
                                         contextCred=sshCred,
                                         dataCollectors=[DataCollector(source="/home/hpcc-user",
                                                                      destination=slaveOutDir + "home_dir",
                                                                      sshCred=sshCred),
                                                         DataCollector(source=collectdSrc,
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

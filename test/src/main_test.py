import sys
import os

from d2c.Application import Application
from d2c.data.DAO import DAO
from d2c.model.Role import Role
from d2c.model.Deployment import Deployment
from d2c.model.InstanceType import InstanceType
from d2c.AMITools import AMIToolsFactory
from d2c.data.CredStore import CredStore
from d2c.model.Cloud import Cloud
from d2c.model.Kernel import Kernel
from d2c.model.AMI import AMI
from copy import copy


from TestConfig import TestConfig

def main(argv=None):
    
    SQLITE_FILE = "%s/.d2c_test/d2c_db.sqlite" % os.path.expanduser('~') 
    if os.path.exists(SQLITE_FILE):
        print "Deleting existing DB"
        os.unlink(SQLITE_FILE)
        
    dao = DAO(SQLITE_FILE)
    credStore = CredStore(dao)
    dao.setCredStore(credStore)
    
    conf = TestConfig("/home/willmore/test.conf")
    
    dao.saveConfiguration(conf)
    
    dao.addAWSCred(conf.awsCred)
    
    dao.addSourceImage("/foobar/vm.vdi")
    
    clouds = [Cloud("SciCloud", 
                        "http://172.17.36.21:8773/services/Eucalyptus",
                        "/home/willmore/Downloads/cloud-cert.pem",
                        "http://172.17.36.21:8773/services/Eucalyptus",
                        [Kernel("aki-123", Kernel.ARCH_X86_64, "/foo/bar")],
                        instanceTypes=[copy(InstanceType.T1_MICRO)]
                        ),
                Cloud("eu-west-1", "https://eu-west-1.ec2.amazonaws.com", "https://s3.amazonaws.com","/opt/EC2_TOOLS/etc/ec2/amitools/cert-ec2.pem",
                      instanceTypes=[copy(InstanceType.T1_MICRO), copy(InstanceType.M1_SMALL), copy(InstanceType.M1_LARGE), copy(InstanceType.M1_XLARGE)]),
                Cloud("us-west-1", "https://us-west-1.ec2.amazonaws.com", "https://s3.amazonaws.com", "/opt/EC2_TOOLS/etc/ec2/amitools/cert-ec2.pem",
                      instanceTypes=[copy(InstanceType.T1_MICRO), copy(InstanceType.M1_SMALL), copy(InstanceType.M1_LARGE), copy(InstanceType.M1_XLARGE)])]
    
    for cloud in clouds:
        dao.saveCloud(cloud)
    
    ami = AMI("ami-47cefa33", "/foobar/vm.vdi", clouds[1])
    dao.addAMI(ami)
        
    dao.saveDeployment(Deployment("dummyDep", 
                                  cloud=clouds[1],
                                  awsCred=conf.awsCred,
                                  roles=[Role("loner", ami, 1, InstanceType.T1_MICRO )]))
   
   
   
   
    app = Application(dao, AMIToolsFactory())
    app.MainLoop()

if __name__ == "__main__":
    sys.exit(main())

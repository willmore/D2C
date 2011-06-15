import sys
import os

from d2c.Application import Application
from d2c.data.DAO import DAO
from d2c.model.Role import Role
from d2c.model.InstanceType import InstanceType
from d2c.model.Deployment import Deployment
from d2c.model.Cloud import Cloud
from d2c.model.Kernel import Kernel
from d2c.model.AMI import AMI
from d2c.data.CredStore import CredStore
from d2c.AMITools import AMITools, AMIToolsFactory
from TestConfig import TestConfig
from mockito import *

    
class DummyConn:
    
    def __init__(self):
        self.num = 0
        self.instances = []
        self.reservations = {}
    
    def get_all_instances(self, ids = None, filters={}):

        return filter(lambda r: r.id in filters['reservation-id'], 
                      self.reservations.values())
    
    def run_instances(self, *args, **kwargs):
        r = DummyReservation(kwargs['min_count'])
        self.reservations[r.id] = r
        return r
    
    def setState(self, state):
        for i in self.instances:
            i.state = state 
        
        for r in self.reservations.values():
            r.state = state
            
class DummyReservation:
    
    ctr = 0
    
    def __init__(self, count, id=None):
        self.instances = [DummyInstance(None) for _ in range(count)]
        self.id = id if id is not None else 'r-dummy_%d' % self.ctr 
        self.ctr += self.ctr
    
    def setState(self, state):
        for i in self.instances:
            i.state = state 
        
    def update(self):
        pass

class DummyInstance():
    
    def __init__(self, id):
        self.id = id
        self.state = 'pending'
        self.key_name = 'dummy_key_name'
        
    def update(self):
        pass


def main(argv=None):
    
    sqlFile = "%s/.d2c_test/main_test_stub.sqlite" % os.path.expanduser('~') 
    if os.path.exists(sqlFile):
        print "Deleting existing DB"
        os.unlink(sqlFile)
    dao = DAO(sqlFile)
    
    conf = TestConfig("/home/willmore/test.conf")   
    dao.saveConfiguration(conf)
    
    #ec2ConnFactory = mock(EC2ConnectionFactory)
    #when(ec2ConnFactory).getConnection().thenReturn(DummyConn())
    
    dao.addAWSCred(conf.awsCred)
    
    dao.setCredStore(CredStore(dao))
    
    dao.addSourceImage("/foobar/vm.vdi")
     
    for cloud in [Cloud("SciCloud", 
                        "http://172.17.36.21:8773/services/Eucalyptus",
                        "/home/willmore/Downloads/cloud-cert.pem",
                        "http://172.17.36.21:8773/services/Eucalyptus",
                        [Kernel("aki-123", Kernel.ARCH_X86_64, "/foo/bar")]
                        ),
                Cloud("eu-west-1", "https://eu-west-1.amazonaws.com", "https://s3.amazonaws.com","/opt/EC2_TOOLS/etc/ec2/amitools/cert-ec2.pem"),
                Cloud("us-west-1", "https://us-west-1.amazonaws.com", "https://s3.amazonaws.com", "/opt/EC2_TOOLS/etc/ec2/amitools/cert-ec2.pem")]:
        dao.saveCloud(cloud)
    
    cloud = dao.getClouds()[0]
    ami = AMI("ami-47cefa33", "/foobar/vm.vdi", cloud)
    dao.addAMI(ami)
    
    deployment = Deployment("dummyDep", 
                            roles=[Role("loner", ami, 1, InstanceType.T1_MICRO)],
                            awsCred=conf.awsCred,
                            cloud=cloud)
    
    dao.saveDeployment(deployment)
       
    
    mockAMIFactory = mock(AMIToolsFactory)
    mockAMITools = mock(AMITools)
    when(mockAMIFactory).getAMITools(any()).thenReturn(mockAMITools)
    when(mockAMITools).getArch(any()).thenReturn(Kernel.ARCH_X86_64)
    
    app = Application(dao, mockAMIFactory)
    app.MainLoop()

if __name__ == "__main__":
    sys.exit(main())

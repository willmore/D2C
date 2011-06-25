import sys
import os

from d2c.Application import Application
from d2c.data.DAO import DAO
from d2c.model.Role import Role
from d2c.model.InstanceType import InstanceType, Architecture
from d2c.model.Deployment import Deployment
from d2c.model.Cloud import Cloud
from d2c.model.Kernel import Kernel
from d2c.model.AMI import AMI
from d2c.data.CredStore import CredStore
from d2c.AMITools import AMITools, AMIToolsFactory
from d2c.model.UploadAction import UploadAction
from TestConfig import TestConfig
from mockito import *
from copy import copy
import boto
from threading import Thread
import time
from d2c.model.SSHCred import SSHCred
from d2c.model.DataCollector import DataCollector
from d2c.model.SourceImage import SourceImage
    
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
        
        class RunThread(Thread):
        
            def __init__(self, dummyConn):
                Thread.__init__(self)
                self.dummyConn = dummyConn
            
            def run(self):
                print "Starting thread"
                for state in ['running', 'terminated']:
                    time.sleep(60)
                    self.dummyConn.setState(state)
        
        RunThread(self).start()
        return r
    
    def setState(self, state):
        
        for r in self.reservations.values():
            r.setState(state)
            
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
        self.private_ip_address = "0.0.0.0"
        
    def update(self):
        pass

    def stop(self):
        pass

def main(argv=None):
    
    sqlFile = "%s/.d2c_test/main_test_stub.sqlite" % os.path.expanduser('~') 
    if os.path.exists(sqlFile):
        print "Deleting existing DB"
        os.unlink(sqlFile)
        
    mockBoto = mock(boto)
    #when(mockBoto).connect_ec2(any(),any(),any(),any(),any(),any()).thenReturn(DummyConn())
    
    def mock_connect_ec2(*args, **kwargs):
        return DummyConn()
    
    mockBoto.connect_ec2 = mock_connect_ec2
   
    dao = DAO(sqlFile, mockBoto)
    
    conf = TestConfig("/home/willmore/test.conf")   
    dao.saveConfiguration(conf)
    
    dao.addAWSCred(conf.awsCred)
    
    dao.setCredStore(CredStore(dao))
    srcImg = SourceImage("/foobar/vm.vdi")
    dao.add(srcImg)
    
    
     
    
     
    clouds = [Cloud("SciCloud", 
                        "http://172.17.36.21:8773/services/Eucalyptus",
                        "/home/willmore/Downloads/cloud-cert.pem",
                        "http://172.17.36.21:8773/services/Eucalyptus",
                        kernels=[Kernel("aki-123", Kernel.ARCH_X86_64, "/foo/bar")],
                        instanceTypes=get_instance_types(dao)
                        ),
                Cloud("eu-west-1", "https://eu-west-1.ec2.amazonaws.com", "https://s3.amazonaws.com","/opt/EC2_TOOLS/etc/ec2/amitools/cert-ec2.pem",
                      instanceTypes=get_instance_types(dao)),
                Cloud("us-west-1", "https://us-west-1.ec2.amazonaws.com", "https://s3.amazonaws.com", "/opt/EC2_TOOLS/etc/ec2/amitools/cert-ec2.pem",
                      get_instance_types(dao))]

    for cloud in clouds:
        dao.saveCloud(cloud)
    
    cloud = dao.getClouds()[0]
    ami = AMI("ami-47cefa33", srcImg, cloud)
    dao.addAMI(ami)
    
    for a in [Architecture('x86'), Architecture('x86_64')]:
        dao.add(a)
        
        
    for instance in []:
        for cloud in clouds:
            instance.cloud = cloud
            dao.addInstanceType(instance)
    
    deployment = Deployment("dummyDep", 
                            roles=[Role("loner", ami, 1, cloud.instanceTypes[0],
                                        startActions=[UploadAction("/tmp/foobar", "/tmp/foobar", mock(SSHCred))], 
                                        dataCollectors=[DataCollector("/tmp", mock(SSHCred))]
                                        
                                        )],
                            awsCred=conf.awsCred,
                            cloud=cloud)
    
    dao.saveDeployment(deployment)
       
    
    mockAMIFactory = mock(AMIToolsFactory)
    mockAMITools = mock(AMITools)
    when(mockAMIFactory).getAMITools(any()).thenReturn(mockAMITools)
    when(mockAMITools).getArch(any()).thenReturn(Kernel.ARCH_X86_64)
    when(mockAMITools).registerAMI(any(), any(), any()).thenReturn("foobarami")
    
    print dao.getAMIs()
    
    app = Application(dao, mockAMIFactory)
    app.MainLoop()
    
def get_instance_types(dao):
        
    X86 = dao.getArchitecture('X86')
    X86_64 = dao.getArchitecture('X86_64')
    
    return [InstanceType('t1.micro', 2, 2, 613, 0, (X86, X86_64), 0.025),
            InstanceType('m1.small', 2, 1, 1700, 160, (X86,), 0.095),
            InstanceType('m1.large', 2, 2, 7500, 850, (X86_64,), 0.038),
            InstanceType('m1.xlarge', 2, 4, 15000, 850, (X86_64,), 0.76)]

if __name__ == "__main__":
    sys.exit(main())

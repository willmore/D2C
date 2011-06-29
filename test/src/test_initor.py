from d2c.model.Role import Role
from d2c.model.InstanceType import InstanceType, Architecture
from d2c.model.Deployment import Deployment
from d2c.model.Cloud import Cloud
from d2c.model.Kernel import Kernel
from d2c.model.AMI import AMI
from d2c.data.CredStore import CredStore
from d2c.model.UploadAction import UploadAction
from TestConfig import TestConfig
from mockito import *

from d2c.model.SSHCred import SSHCred
from d2c.model.DataCollector import DataCollector
from d2c.model.SourceImage import SourceImage
import tempfile

def init_db(dao, confFile):
    conf = TestConfig(confFile)   
    dao.saveConfiguration(conf)
    
    dao.addAWSCred(conf.awsCred)
    
    dao.setCredStore(CredStore(dao))
    srcImg = SourceImage("/home/willmore/images/worker.vdi")
    dao.add(srcImg) 
    archs = [Architecture('x86'), Architecture('x86_64')]
     
    for a in archs:
        dao.add(a)
         
    clouds = [Cloud(name="SciCloud", 
                        serviceURL="http://172.17.36.21:8773/services/Eucalyptus",
                        ec2Cert="/home/willmore/.euca/cloud-cert.pem",
                        storageURL="http://172.17.36.21:8773/services/Walrus",
                        kernels=[Kernel("eki-3EB4165A", archs[1], "internal://ami_data/kernels/2.6.35-24-virtual-x86_64.tar")],
                        instanceTypes=get_instance_types(dao)
                        ),
                Cloud("eu-west-1", 
                      "https://eu-west-1.ec2.amazonaws.com", 
                      "https://s3.amazonaws.com",
                      "/opt/EC2_TOOLS/etc/ec2/amitools/cert-ec2.pem",
                      instanceTypes=get_instance_types(dao)),
                Cloud("us-west-1", 
                      "https://us-west-1.ec2.amazonaws.com", 
                      "https://s3.amazonaws.com", 
                      "/opt/EC2_TOOLS/etc/ec2/amitools/cert-ec2.pem",
                      get_instance_types(dao))]

    for cloud in clouds:
        dao.add(cloud)
    
    #ami = AMI("ami-47cefa33", srcImg, cloud)
    cloud = clouds[0]
    ami = AMI("emi-58091682", cloud, srcImg, kernel=cloud.kernels[0])
    dao.addAMI(ami)
     
    for instance in []:
        for cloud in clouds:
            instance.cloud = cloud
            dao.addInstanceType(instance)
    
    sshCred = SSHCred("key", "root", "/home/willmore/.euca/key.private")
    dao.add(sshCred)
    
    tmpSrc = tempfile.NamedTemporaryFile(delete=False)
    tmpSrc.write("blah")
    
    deployment = Deployment("dummyDep", 
                            dataDir="/home/willmore/.d2c_test/deployments/dummyDep",
                            roles=[Role("loner", ami, 1, cloud.instanceTypeByName("m1.small"),
                                        uploadActions=[UploadAction(tmpSrc.name, "/tmp/foobar", sshCred)], 
                                        dataCollectors=[DataCollector("/tmp", sshCred)],
                                        contextCred=sshCred,
                                        launchCred=sshCred
                                        )],
                            awsCred=conf.awsCred,
                            cloud=cloud)
    
    dao.save(deployment)
    
def get_instance_types(dao):
        
    X86 = dao.getArchitecture('x86')
    X86_64 = dao.getArchitecture('x86_64')
    
    return [InstanceType('t1.micro', 2, 2, 613, 0, (X86, X86_64), 0.025),
            InstanceType('m1.small', 2, 1, 1700, 160, (X86,), 0.095),
            InstanceType('m1.large', 2, 2, 7500, 850, (X86_64,), 0.038),
            InstanceType('m1.xlarge', 2, 4, 15000, 850, (X86_64,), 0.76)]
from d2c.model.InstanceType import InstanceType, Architecture
from d2c.model.Cloud import EC2Cloud, DesktopCloud
from d2c.model.Kernel import Kernel
from d2c.data.CredStore import CredStore
from d2c.model.UploadAction import UploadAction
from d2c.model.Ramdisk import Ramdisk
from d2c.model.SSHCred import SSHCred
from d2c.model.DataCollector import DataCollector
from d2c.model.SourceImage import Image, DesktopImage, AMI
from TestConfig import TestConfig
from d2c.model.DeploymentTemplate import DeploymentTemplate, RoleTemplate
from d2c.model.FileExistsFinishedCheck import FileExistsFinishedCheck

from mockito import *
import tempfile

def init_db(dao, confFile):
    conf = TestConfig(confFile)   
    dao.saveConfiguration(conf)
    
    dao.addAWSCred(conf.awsCred)
    
    dao.setCredStore(CredStore(dao))
   
    archs = [Architecture('x86'), Architecture('x86_64')]
     
    for a in archs:
        dao.add(a)

    
    
    clouds = [EC2Cloud(None, 
                       name="SciCloud", 
                        serviceURL="http://172.17.36.21:8773/services/Eucalyptus",
                        ec2Cert="/home/willmore/.euca/cloud-cert.pem",
                        storageURL="http://172.17.36.21:8773/services/Walrus",
                        kernels=[Kernel("eki-3EB4165A", archs[1], "internal://ami_data/kernels/2.6.35-24-virtual-x86_64.tar")],
                        instanceTypes=get_instance_types(dao)
                        ),
              EC2Cloud(
                       None,
                       "eu-west-1", 
                      "https://eu-west-1.ec2.amazonaws.com", 
                      "https://s3.amazonaws.com",
                      "/opt/EC2_TOOLS/etc/ec2/amitools/cert-ec2.pem",
                      instanceTypes=get_instance_types(dao)),
              EC2Cloud(
                       None,
                       "us-west-1", 
                      "https://us-west-1.ec2.amazonaws.com", 
                      "https://s3.amazonaws.com", 
                      "/opt/EC2_TOOLS/etc/ec2/amitools/cert-ec2.pem",
                      get_instance_types(dao)),
              DesktopCloud(None, "VirtualBox")]

    for cloud in clouds:
        dao.add(cloud)
        
    deskImg = DesktopImage(None, None, clouds[3], "/home/willmore/images/worker.vdi")
    myWorkerImg = Image(None, "My Worker", deskImg, [deskImg])
    
    dao.add(myWorkerImg) 
    
    cloud = clouds[0]
    
    ramdisk = Ramdisk("eri-83141744", cloud, archs[1])
    dao.add(ramdisk)

    ami = AMI(None, myWorkerImg, "emi-77180ECD", cloud)
    dao.addAMI(ami)
     
    for instance in []:
        for cloud in clouds:
            instance.cloud = cloud
            dao.addInstanceType(instance)
    
    sshCred = SSHCred(None, "key", "root", "/home/willmore/.euca/key.private")
    dao.add(sshCred)
    
    tmpSrc = tempfile.NamedTemporaryFile(delete=False)
    tmpSrc.write("blah")
    
    dTemplate = DeploymentTemplate(None, 
                                   "dummyDep", 
                                   dataDir="/home/willmore/.d2c_test/deployments/dummyDep",
                                   roleTemplates=[RoleTemplate(
                                                        None,
                                                        name="Loner",
                                                        image = myWorkerImg, 
                                                        dataCollectors=[DataCollector("/tmp/d2c.context")],
                                                        finishedChecks=[FileExistsFinishedCheck("/tmp/d2c.context")],
                                                        contextCred=sshCred,
                                                        launchCred=sshCred
                                        )])
    
    dao.add(dTemplate)
    sciCloud = clouds[0]
    vbCloud = clouds[3]
    
    m1large = None;
    for inst in sciCloud.instanceTypes:
        if inst.name == "m1.large":
            m1large = inst
            break
    
    roleReq = {}
    for roleTemp in dTemplate.roleTemplates:
        roleReq[roleTemp] = (ami, m1large, 1)
    deployment = dTemplate.createDeployment(sciCloud, roleReq, conf.awsCred)
  
    dao.add(deployment)
  
    roleReq = {}
    for roleTemp in dTemplate.roleTemplates:
        roleReq[roleTemp] = (deskImg, None, 2)
    deployment = dTemplate.createDeployment(vbCloud, roleReq)
    
    dao.add(deployment)
    
def get_instance_types(dao):
        
    X86 = dao.getArchitecture('x86')
    X86_64 = dao.getArchitecture('x86_64')
    
    return [InstanceType('t1.micro', 2, 2, 613, 0, (X86, X86_64), 0.025),
            InstanceType('c1.medium', 2, 1, 1700, 160, (X86,), 0.095),
            InstanceType('c1.xlarge', 2, 1, 1700, 160, (X86,), 0.095),
            InstanceType('m1.small', 2, 1, 1700, 160, (X86,), 0.095),
            InstanceType('m1.large', 2, 2, 7500, 850, (X86_64,), 0.038),
            InstanceType('m1.xlarge', 2, 4, 15000, 850, (X86_64,), 0.76)]
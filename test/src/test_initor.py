from d2c.model.InstanceType import InstanceType, Architecture
from d2c.model.Cloud import EC2Cloud, DesktopCloud
from d2c.model.Kernel import Kernel
from d2c.model.AMI import AMI
from d2c.data.CredStore import CredStore
from d2c.model.UploadAction import UploadAction
from d2c.model.Ramdisk import Ramdisk
from TestConfig import TestConfig
from d2c.model.DeploymentTemplate import DeploymentTemplate, RoleTemplate
from mockito import *

from d2c.model.SSHCred import SSHCred
from d2c.model.DataCollector import DataCollector
from d2c.model.SourceImage import Image, DesktopImage
import tempfile

def init_db(dao, confFile):
    conf = TestConfig(confFile)   
    dao.saveConfiguration(conf)
    
    dao.addAWSCred(conf.awsCred)
    
    dao.setCredStore(CredStore(dao))
    deskImg = DesktopImage(None, None, "/home/willmore/images/worker.vdi")
    myWorkerImg = Image(None, "My Worker", deskImg, [deskImg])
    
     
    dao.add(myWorkerImg) 
    archs = [Architecture('x86'), Architecture('x86_64')]
     
    for a in archs:
        dao.add(a)
         
    clouds = [EC2Cloud(name="SciCloud", 
                        serviceURL="http://172.17.36.21:8773/services/Eucalyptus",
                        ec2Cert="/home/willmore/.euca/cloud-cert.pem",
                        storageURL="http://172.17.36.21:8773/services/Walrus",
                        kernels=[Kernel("eki-B482178C", archs[1], "internal://ami_data/kernels/2.6.27.21-0.1-xen-modules.tgz"),
                                 Kernel("eki-3EB4165A", archs[1], "internal://ami_data/kernels/2.6.35-24-virtual-x86_64.tgz")],
                        instanceTypes=get_instance_types(dao)
                        ),
              EC2Cloud("eu-west-1", 
                      "https://eu-west-1.ec2.amazonaws.com", 
                      "https://s3.amazonaws.com",
                      "/opt/EC2_TOOLS/etc/ec2/amitools/cert-ec2.pem",
                      instanceTypes=get_instance_types(dao)),
              EC2Cloud("us-west-1", 
                      "https://us-west-1.ec2.amazonaws.com", 
                      "https://s3.amazonaws.com", 
                      "/opt/EC2_TOOLS/etc/ec2/amitools/cert-ec2.pem",
                      get_instance_types(dao)),
              DesktopCloud("VirtualBox")]

    for cloud in clouds:
        dao.add(cloud)
    
    #ami = AMI("ami-47cefa33", srcImg, cloud)
    cloud = clouds[0]
    ramdisk = Ramdisk("eri-AEC21764", cloud, archs[1])
    dao.add(ramdisk)
    ramdisk = Ramdisk("eri-83141744", cloud, archs[1])
    dao.add(ramdisk)
    ami = AMI(None, myWorkerImg, "emi-58091682", cloud, kernel=cloud.kernels[0], ramdisk=ramdisk)
    dao.addAMI(ami)
     
    for instance in []:
        for cloud in clouds:
            instance.cloud = cloud
            dao.addInstanceType(instance)
    
    sshCred = SSHCred("key", "root", "/home/willmore/.euca/key.private")
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
                                                        uploadActions=[UploadAction(tmpSrc.name, "/tmp/foobar", sshCred)], 
                                                        dataCollectors=[DataCollector("/tmp", sshCred)],
                                                        contextCred=sshCred,
                                                        launchCred=sshCred
                                        )])

    
    dao.add(dTemplate)
    ec2Cloud = clouds[0]
    vbCloud = clouds[3]
    
    roleReq = {}
    for roleTemp in dTemplate.roleTemplates:
        roleReq[roleTemp] = (ami, ec2Cloud.instanceTypes[0], 2)
    deployment = dTemplate.createDeployment(ec2Cloud, roleReq)
  
    roleReq = {}
    for roleTemp in dTemplate.roleTemplates:
        roleReq[roleTemp] = (deskImg, None, 2)
    deployment = dTemplate.createDeployment(vbCloud, roleReq)
    
    dao.add(deployment)
    
def get_instance_types(dao):
        
    X86 = dao.getArchitecture('x86')
    X86_64 = dao.getArchitecture('x86_64')
    
    return [InstanceType('t1.micro', 2, 2, 613, 0, (X86, X86_64), 0.025),
            InstanceType('m1.small', 2, 1, 1700, 160, (X86,), 0.095),
            InstanceType('m1.large', 2, 2, 7500, 850, (X86_64,), 0.038),
            InstanceType('m1.xlarge', 2, 4, 15000, 850, (X86_64,), 0.76)]
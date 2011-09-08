from d2c.model.InstanceType import InstanceType, Architecture
from d2c.model.Cloud import EC2Cloud, DesktopCloud
from d2c.model.Kernel import Kernel
from d2c.data.CredStore import CredStore
from d2c.model.Ramdisk import Ramdisk
from d2c.model.SSHCred import SSHCred
from d2c.model.DataCollector import DataCollector
from d2c.model.SourceImage import Image, DesktopImage, AMI
from TestConfig import TestConfig
from d2c.model.DeploymentTemplate import DeploymentTemplate, RoleTemplate
from d2c.model.FileExistsFinishedCheck import FileExistsFinishedCheck
from d2c.model.Action import StartAction
from d2c.model.AsyncAction import AsyncAction
from d2c.model.Deployment import StateEvent, DeploymentState
import tempfile
import time

from numpy import true_divide

def init_db(dao, confFile):
    '''
    Pre-fill DB with usefull entities for manual testing.
    '''
    
    conf = TestConfig(confFile)   
    dao.saveConfiguration(conf)
    
    dao.addAWSCred(conf.awsCred)
    
    dao.setCredStore(CredStore(dao))
    X86 = Architecture('x86')
    X86_64 = Architecture('x86_64')
    
    archs = [X86, X86_64]
     
    for a in archs:
        dao.add(a)
    
    boto=dao.botoModule
    
    clouds = [EC2Cloud(None, 
                       name="SciCloud", 
                        serviceURL="http://172.17.36.21:8773/services/Eucalyptus",
                        ec2Cert="/home/willmore/.euca/cloud-cert.pem",
                        storageURL="http://172.17.36.21:8773/services/Walrus",
                        kernels=[Kernel("eki-3EB4165A", archs[1], "internal://ami_data/kernels/2.6.35-24-virtual-x86_64.tar")],
                        instanceTypes=[
                                       InstanceType('m1.small', 2, 1, 256, 2, (X86,), 0.0),
                                       InstanceType('m1.large', 2, 2, 1792, 15, (X86_64,), 0.0),
                                       InstanceType('m1.xlarge', 2, 1, 1792, 20, (X86_64,), 0.0),
                                       InstanceType('c1.medium', 2, 2, 512, 5, (X86_64,), 0.0),
                                       InstanceType('c1.xlarge', 2, 4, 1792, 20, (X86_64,), 0.0)],
                        botoModule=boto),
              EC2Cloud(
                       None,
                       "eu-west-1", 
                      "https://eu-west-1.ec2.amazonaws.com", 
                      "https://s3.amazonaws.com",
                      "/opt/EC2_TOOLS/etc/ec2/amitools/cert-ec2.pem",
                      instanceTypes=get_instance_types(dao),
                      botoModule=boto),
              EC2Cloud(
                       None,
                       "us-west-1", 
                      "https://us-west-1.ec2.amazonaws.com", 
                      "https://s3.amazonaws.com", 
                      "/opt/EC2_TOOLS/etc/ec2/amitools/cert-ec2.pem",
                      botoModule=boto,
                      instanceTypes=get_instance_types(dao)),
              DesktopCloud(None, "VirtualBox", [InstanceType('defaultType', 1, 1, 512, 0, (dao.getArchitecture('x86_64'),), 0.0)])]


    for cloud in clouds:
        dao.add(cloud)
        
    deskImg = DesktopImage(None, None, clouds[3], time.time(), 2000000, 8000000, "/home/willmore/images/worker.vdi")
    myWorkerImg = Image(None, "Worker", deskImg, [deskImg])
    
    dao.add(myWorkerImg) 
    
    cloud = clouds[0]
    
    ramdisk = Ramdisk("eri-83141744", cloud, archs[1])
    dao.add(ramdisk)

    ami = AMI(None, myWorkerImg, "emi-77180ECD", cloud, time.time(), 8000000)
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
                                                        name="Worker",
                                                        image = myWorkerImg, 
                                                        dataCollectors=[DataCollector("/tmp/pingtest.out")],
                                                        finishedChecks=[FileExistsFinishedCheck("/tmp/pingtest.out")],
                                                        startActions=[StartAction("echo \"Host *\" >> /home/dirac/.ssh/config; echo \"    StrictHostKeyChecking no\" >> /home/dirac/.ssh/config;", None)],
                                                        asyncStartActions=[AsyncAction("sudo -u dirac mpirun -wdir /home/dirac -np 2 -hostfile /tmp/d2c.context hostname > /tmp/pingtest.out", None)]
                                                    
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
        roleReq[roleTemp] = (ami, m1large, 2)
    deployment = dTemplate.createDeployment(sciCloud, roleReq, conf.awsCred, 35)
  
    dao.add(deployment)
  
    roleReq = {}
    for roleTemp in dTemplate.roleTemplates:
        roleReq[roleTemp] = (deskImg, vbCloud.instanceTypes[0], 2)
    
    
    deployment = dTemplate.createDeployment(vbCloud, roleReq, None, 60)
    #deployment.problemSize = 60
    
    dao.add(deployment)

    '''
    Populate some real resuls from HPCC tests
    '''
    
    hpccTemplate = DeploymentTemplate(None, 
                                   "HPPC", 
                                   dataDir="/home/willmore/.d2c_test/deployments/hppc",
                                   roleTemplates=[RoleTemplate(
                                                        None,
                                                        name="Master",
                                                        image = myWorkerImg, 
                                                        dataCollectors=[DataCollector("/tmp/pingtest.out")],
                                                        finishedChecks=[FileExistsFinishedCheck("/tmp/pingtest.out")],
                                                        startActions=[StartAction("for ip in `cat /tmp/d2c.context`; do ping $ip -c 10; done > /tmp/pingtest.out", None)]
                                                    
                                                  ),
                                                  RoleTemplate(
                                                        None,
                                                        name="Slave",
                                                        image = myWorkerImg, 
                                                        dataCollectors=[DataCollector("/tmp/pingtest.out")],
                                                        finishedChecks=[FileExistsFinishedCheck("/tmp/pingtest.out")],
                                                        startActions=[StartAction("for ip in `cat /tmp/d2c.context`; do ping $ip -c 10; done > /tmp/pingtest.out", None)]
                                                    
                                        )])
    master = hpccTemplate.roleTemplates[0]
    slave = hpccTemplate.roleTemplates[1]
    '''
    N probSize cpuCount   time 
    9268 85895824 1 1 2 0.07 613541600

9268 85895824 2 1 2 0.04 536848900

10000 100000000 1 2 2 0.08 625000000

10000 100000000 2 2 2 0.05 500000000

14142 199996164 1 2 2 0.27 370363267

14142 199996164 2 2 2 0.17 294112006

19364 374964496 1 3 2 0.76 246687168

19364 374964496 2 3 2 0.51 183806125
    '''
    
    
    def sec(hrFrac):
        return 3600 * hrFrac
    
    roleReq = {master : (deskImg, vbCloud.instanceTypes[0], 1),
               slave : (deskImg, vbCloud.instanceTypes[0], 1)}
    d = hpccTemplate.createDeployment(vbCloud, roleReq, conf.awsCred, 85.895824)
    
    '''HPCC virtual box results'''
    roleReq = {master : (deskImg, vbCloud.instanceTypes[0], 1)}
    d = hpccTemplate.createDeployment(vbCloud, roleReq, conf.awsCred, 85.895824)
    d.state = DeploymentState.COMPLETED
    d.stateEvents = [StateEvent(DeploymentState.ROLES_STARTED, 0),
                    StateEvent(DeploymentState.JOB_COMPLETED, sec(0.07))]
    d.maxMemory = 124
    dao.add(d)
    
    roleReq = {master : (deskImg, vbCloud.instanceTypes[0], 1),
               slave : (deskImg, vbCloud.instanceTypes[0], 1)}
    d = hpccTemplate.createDeployment(vbCloud, roleReq, conf.awsCred, 85.89582)
    d.state = DeploymentState.COMPLETED
    d.stateEvents = [StateEvent(DeploymentState.ROLES_STARTED, 0),
                    StateEvent(DeploymentState.JOB_COMPLETED, sec(0.04))]
    d.maxMemory = 124
    dao.add(d)
    
    roleReq = {master : (deskImg, vbCloud.instanceTypes[0], 1)}
    d = hpccTemplate.createDeployment(vbCloud, roleReq, conf.awsCred, 100.)
    d.state = DeploymentState.COMPLETED
    d.stateEvents = [StateEvent(DeploymentState.ROLES_STARTED, 0),
                    StateEvent(DeploymentState.JOB_COMPLETED, sec(0.08))]
    d.maxMemory = 126
    dao.add(d)
    
    roleReq = {master : (deskImg, vbCloud.instanceTypes[0], 1),
               slave : (deskImg, vbCloud.instanceTypes[0], 1)}
    d = hpccTemplate.createDeployment(vbCloud, roleReq, conf.awsCred, 100.)
    d.state = DeploymentState.COMPLETED
    d.stateEvents = [StateEvent(DeploymentState.ROLES_STARTED, 0),
                    StateEvent(DeploymentState.JOB_COMPLETED, sec(0.05))]
    d.maxMemory = 127
    dao.add(d)
    
    roleReq = {master : (deskImg, vbCloud.instanceTypes[0], 1)}
    d = hpccTemplate.createDeployment(vbCloud, roleReq, conf.awsCred, 199.996164)
    d.state = DeploymentState.COMPLETED
    d.stateEvents = [StateEvent(DeploymentState.ROLES_STARTED, 0),
                    StateEvent(DeploymentState.JOB_COMPLETED, sec(0.27))]
    d.maxMemory = 258
    dao.add(d)
    
    roleReq = {master : (deskImg, vbCloud.instanceTypes[0], 1),
               slave : (deskImg, vbCloud.instanceTypes[0], 1)}
    d = hpccTemplate.createDeployment(vbCloud, roleReq, conf.awsCred, 199.996164)
    d.state = DeploymentState.COMPLETED
    d.stateEvents = [StateEvent(DeploymentState.ROLES_STARTED, 0),
                    StateEvent(DeploymentState.JOB_COMPLETED, sec(0.17))]
    d.maxMemory = 243
    dao.add(d)
    
    roleReq = {master : (deskImg, vbCloud.instanceTypes[0], 1)}
    d = hpccTemplate.createDeployment(vbCloud, roleReq, conf.awsCred, 374.964496)
    d.state = DeploymentState.COMPLETED
    d.stateEvents = [StateEvent(DeploymentState.ROLES_STARTED, 0),
                    StateEvent(DeploymentState.JOB_COMPLETED, sec(0.76))]
    d.maxMemory = 340
    dao.add(d)
    
    roleReq = {master : (deskImg, vbCloud.instanceTypes[0], 1),
               slave : (deskImg, vbCloud.instanceTypes[0], 1)}
    d = hpccTemplate.createDeployment(vbCloud, roleReq, conf.awsCred, 374.964496)
    d.state = DeploymentState.COMPLETED
    d.stateEvents = [StateEvent(DeploymentState.ROLES_STARTED, 0),
                     StateEvent(DeploymentState.JOB_COMPLETED, sec(0.51))]
    d.maxMemory = 345
    dao.add(d)
    
    '''HPCC EC2 results'''
    
def get_instance_types(dao):
        
    X86 = dao.getArchitecture('x86')
    X86_64 = dao.getArchitecture('x86_64')
    
    return [#InstanceType('t1.micro', 2, 2, 613, 0, (X86, X86_64), 0.025),
            InstanceType('c1.medium', 2, 1, 1700, 160, (X86,), 0.19),
            InstanceType('c1.xlarge', 2, 1, 1700, 160, (X86,), 0.76),
            InstanceType('m1.small', 2, 1, 1700, 160, (X86,), 0.095),
            InstanceType('m1.large', 2, 2, 7500, 850, (X86_64,), 0.38),
            InstanceType('m1.xlarge', 2, 4, 15000, 850, (X86_64,), 0.76)]
    

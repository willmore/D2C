from pylab import *
from numpy import *
import numpy
from scipy.optimize import leastsq
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
from d2c.model.AWSCred import AWSCred
from d2c.model.Deployment import DeploymentState, StateEvent
from d2c.model.CompModel import PolyCompModel
import boto


from d2c.model.CompModel import CompModel

def run():
    deploymentTemp = init()

    model = PolyCompModel(deploymentTemp)
    realProbSize = array([dp.probSize for dp in model.dataPoints])
    times = array([dp.time for dp in model.dataPoints])
    
    plots= []
    plots.append(plot(realProbSize,times,'ro')) 
    labels = ['real']
    
    probSize = linspace(0,200,5)
    
    for cpu in []:
        for count in range(1,5):
            plots.append(plot(probSize, model.modelFunc(probSize, cpu, count)))
            labels.append("%d:%d" % (cpu,count))
    
    legend([p[0] for p in plots], labels)

    show()

def init():
    
    X86 = Architecture('x86')
    X86_64 = Architecture('x86_64')
    
    archs = [X86, X86_64]
    
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
                      instanceTypes=[
                                       InstanceType('m1.small', 2, 1, 256, 2, (X86,), 0.0),
                                       InstanceType('m1.large', 2, 2, 1792, 15, (X86_64,), 0.0),
                                       InstanceType('m1.xlarge', 2, 1, 1792, 20, (X86_64,), 0.0),
                                       InstanceType('c1.medium', 2, 2, 512, 5, (X86_64,), 0.0),
                                       InstanceType('c1.xlarge', 2, 4, 1792, 20, (X86_64,), 0.0)],
                      botoModule=boto),
              EC2Cloud(
                       None,
                       "us-west-1", 
                      "https://us-west-1.ec2.amazonaws.com", 
                      "https://s3.amazonaws.com", 
                      "/opt/EC2_TOOLS/etc/ec2/amitools/cert-ec2.pem",
                      botoModule=boto,
                      instanceTypes=[InstanceType('m1.small', 2, 1, 256, 2, (X86,), 0.0),
                                       InstanceType('m1.large', 2, 2, 1792, 15, (X86_64,), 0.0),
                                       InstanceType('m1.xlarge', 2, 1, 1792, 20, (X86_64,), 0.0),
                                       InstanceType('c1.medium', 2, 2, 512, 5, (X86_64,), 0.0),
                                       InstanceType('c1.xlarge', 2, 4, 1792, 20, (X86_64,), 0.0)]),
              DesktopCloud(None, "VirtualBox", [InstanceType('defaultType', 1, 1, 512, 0, (X86_64,), 0.0)])]
        
    deskImg = DesktopImage(None, None, clouds[3], "/home/willmore/images/worker.vdi")
    myWorkerImg = Image(None, "Worker", deskImg, [deskImg])
    
    cloud = clouds[0]
    

    ami = AMI(None, myWorkerImg, "emi-77180ECD", cloud)
    
    
    dTemplate = DeploymentTemplate(None, 
                                   "dummyDep", 
                                   dataDir="/home/willmore/.d2c_test/deployments/dummyDep",
                                   roleTemplates=[RoleTemplate(
                                                        None,
                                                        name="Worker",
                                                        image = myWorkerImg, 
                                                        dataCollectors=[DataCollector("/tmp/d2c.context")],
                                                        finishedChecks=[FileExistsFinishedCheck("/tmp/d2c.context")],
                                                        #contextCred=sshCred,
                                                        #launchCred=sshCred
                                                    
                                        )])
    
    sciCloud = clouds[0]
    vbCloud = clouds[3]
    
    m1large = None;
    for inst in sciCloud.instanceTypes:
        if inst.name == "m1.large":
            m1large = inst
            break
    
    #deployments = []
    
    roleReq = {}
    for roleTemp in dTemplate.roleTemplates:
        roleReq[roleTemp] = (ami, m1large, 2)
    deployment = dTemplate.createDeployment(sciCloud, roleReq, AWSCred(None, 'foo', 'bar'))
    deployment.problemSize = 35
    deployment.stateEvents = [StateEvent(DeploymentState.ROLES_STARTED, 0),
                              StateEvent(DeploymentState.JOB_COMPLETED, 100)]
    
    #deployments.append(deployment)
    
    roleReq = {}
    for roleTemp in dTemplate.roleTemplates:
        roleReq[roleTemp] = (ami, m1large, 2)
    deployment = dTemplate.createDeployment(sciCloud, roleReq, AWSCred(None, 'foo', 'bar'))
    deployment.problemSize = 70
    deployment.stateEvents = [StateEvent(DeploymentState.ROLES_STARTED, 0),
                              StateEvent(DeploymentState.JOB_COMPLETED, 215)]
    
    #deployments.append(deployment)
    
    return dTemplate
    
run()
    
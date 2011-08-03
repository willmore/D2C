from pylab import *
from numpy import *
import numpy
#from scipy.optimize import leastsq
from d2c.model.InstanceType import InstanceType, Architecture
from d2c.model.Cloud import EC2Cloud, DesktopCloud
from d2c.model.Kernel import Kernel
from d2c.model.DataCollector import DataCollector
from d2c.model.SourceImage import Image, DesktopImage, AMI
from d2c.model.DeploymentTemplate import DeploymentTemplate, RoleTemplate
from d2c.model.FileExistsFinishedCheck import FileExistsFinishedCheck
from d2c.model.AWSCred import AWSCred
from d2c.model.Deployment import DeploymentState, StateEvent
from d2c.model.CompModel import PolyCompModel, AmdahlsCompModel
import boto

import unittest

from d2c.model.CompModel import CompModel, pEstimated, speedUp, runTime, DataPoint

class CompModelTest(unittest.TestCase):

    def testPEstimated(self):
        SU = true_divide(1000,600)
        NP = 2
        p = pEstimated(SU, NP)
        self.assertEquals(0.80, p)
    
    def testSpeedUp(self):
        SU = speedUp(2, 0.895)
        self.assertEquals(round(SU, 2), 1.81)
    
    def testRunTime(self):
        t1 = 188
        p = 0.895
        n = 4
        t = runTime(t1, p, n)
        self.assertEquals(round(t, 2), 61.8)
        
    def testAmdahlModelLinear(self):
        points = [DataPoint(cpuCount=1, cpu=1, time=1000, probSize=35), 
                  DataPoint(cpuCount=2, cpu=1, time=600, probSize=35), 
                  DataPoint(cpuCount=2, cpu=1, time=1200, probSize=70)]
        
        model = AmdahlsCompModel(dataPoints=points, scaleFunction='linear')
        
        self.assertEquals(1000, model.modelFunc(probSize=35, cpu=1, count=1))
        self.assertEquals(600, model.modelFunc(probSize=35, cpu=1, count=2))
        self.assertAlmostEquals(400, model.modelFunc(probSize=35, cpu=1, count=4))
        self.assertAlmostEquals(300, model.modelFunc(probSize=35, cpu=1, count=8))
        
        self.assertEquals(1200, model.modelFunc(probSize=70, cpu=1, count=2))
        self.assertEquals(2400, model.modelFunc(probSize=140, cpu=1, count=2))
        return 
        plots= []
        plots.append(plot([35, 35, 70],[1000, 600, 1200],'ro')) 
        labels = ['real']
    
        probSize = linspace(0,200,5)
    
        for count in range(1,5):
            plots.append(plot(probSize, model.modelFunc(probSize, None, count)))
            labels.append("%d" % count)
    
            legend([p[0] for p in plots], labels)

        show()
        
    def testAmdahlModelLog(self):
        points = [DataPoint(cpuCount=1, cpu=1, time=1000, probSize=35), 
                  DataPoint(cpuCount=2, cpu=1, time=600, probSize=35), 
                  DataPoint(cpuCount=2, cpu=1, time=1200, probSize=70)]
        
        model = AmdahlsCompModel(dataPoints=points, scaleFunction='log')
        
        #self.assertEquals(1000, model.modelFunc(probSize=35, cpu=1, count=1))
        #self.assertEquals(600, model.modelFunc(probSize=35, cpu=1, count=2))
        #self.assertEquals(1200, model.modelFunc(probSize=70, cpu=1, count=2))
        
        plots= []
        plots.append(plot([35, 35, 70],[1000, 600, 1200],'ro')) 
        labels = ['real']
    
        probSize = linspace(0,200,5)
    
        for count in range(1,5):
            plots.append(plot(probSize, model.modelFunc(probSize, None, count)))
            labels.append("%d" % count)
    
            legend([p[0] for p in plots], labels)

        show()
         

def run():
     
     
    points = [DataPoint(cpuCount=1, cpu=1, time=1000, probSize=35), 
              DataPoint(cpuCount=2, cpu=1, time=600, probSize=35), 
              DataPoint(cpuCount=2, cpu=1, time=1200, probSize=70)]
    
    model = AmdahlsCompModel(dataPoints=points)
    
    #self.assertEquals(1000, model.modelFunc(probSize=35, cpu=1, count=1))
    #self.assertEquals(600, model.modelFunc(probSize=35, cpu=1, count=2))
    #self.assertEquals(1200, model.modelFunc(probSize=70, cpu=1, count=2))
    
    plots= []
    plots.append(plot([35, 35, 70],[1000, 600, 1200],'ro')) 
    labels = ['real']
    
    probSize = linspace(0,200,5)
    
    for count in range(1,5):
        plots.append(plot(probSize, model.modelFunc(probSize, None, count)))
        labels.append("%d" % count)
    
        legend([p[0] for p in plots], labels)
    
    show()     
     
    deploymentTemp = init()

    #model = PolyCompModel(deploymentTemp)
    model = AmdahlsCompModel(deploymentTemp)
    global clouds
    costModel = model.costModel(clouds[1])
    
    for size in [400, 800, 1200, 1600]:
        best = costModel(size)
        print size, best
        
    realProbSize = array([dp.probSize for dp in model.dataPoints])
    times = array([dp.time for dp in model.dataPoints])
    
    plots= []
    plots.append(plot(realProbSize,times,'ro')) 
    labels = ['real']
    
    probSize = linspace(0,200,5)
    
    for count in range(1,5):
        plots.append(plot(probSize, model.modelFunc(probSize, None, count)))
        labels.append("%d" % count)
    
    legend([p[0] for p in plots], labels)

    show()
    

def init():
    
    X86 = Architecture('x86')
    X86_64 = Architecture('x86_64')
    
    archs = [X86, X86_64]
    
    global clouds
    
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
                                       InstanceType('m1.small', 2, 1, 256, 2, (X86,), 0.095),
                                       InstanceType('m1.large', 2, 2, 1792, 15, (X86_64,), 0.38),
                                       InstanceType('m1.xlarge', 2, 1, 1792, 20, (X86_64,), 0.76),
                                       InstanceType('c1.medium', 2, 2, 512, 5, (X86_64,), 0.19),
                                       InstanceType('c1.xlarge', 2, 4, 1792, 20, (X86_64,), 0.76)],
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
    m1small = None
    for inst in sciCloud.instanceTypes:
        if inst.name == "m1.large":
            m1large = inst
        if inst.name == "m1.small":
            m1small = inst
    
    #deployments = []
    
    roleReq = {}
    for roleTemp in dTemplate.roleTemplates:
        roleReq[roleTemp] = (ami, m1small, 1)
    deployment = dTemplate.createDeployment(sciCloud, roleReq, AWSCred(None, 'foo', 'bar'))
    deployment.problemSize = 35
    deployment.stateEvents = [StateEvent(DeploymentState.ROLES_STARTED, 0),
                              StateEvent(DeploymentState.JOB_COMPLETED, 1000)]
    
    #deployments.append(deployment)
    
    roleReq = {}
    for roleTemp in dTemplate.roleTemplates:
        roleReq[roleTemp] = (ami, m1small, 2)
    deployment = dTemplate.createDeployment(sciCloud, roleReq, AWSCred(None, 'foo', 'bar'))
    deployment.problemSize = 35
    deployment.stateEvents = [StateEvent(DeploymentState.ROLES_STARTED, 0),
                              StateEvent(DeploymentState.JOB_COMPLETED, 600)]
    
    roleReq = {}
    for roleTemp in dTemplate.roleTemplates:
        roleReq[roleTemp] = (ami, m1small, 2)
    deployment = dTemplate.createDeployment(sciCloud, roleReq, AWSCred(None, 'foo', 'bar'))
    deployment.problemSize = 70
    deployment.stateEvents = [StateEvent(DeploymentState.ROLES_STARTED, 0),
                              StateEvent(DeploymentState.JOB_COMPLETED, 1200)]
    
    #deployments.append(deployment)
    
    return dTemplate
    


if __name__ == "__main__":
    run()
    #import sys;sys.argv = ['', 'Test.testName']
    unittest.main()

    
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
from d2c.model.CompModel import PolyCompModel, AmdahlsCompModel, PolyCompModel2
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
        
        #Assert that the runtime never goes < 0 as num procs approaches inf
        self.assertGreater(runTime(t1, p, 200), 0)
        self.assertGreater(runTime(t1, p, 1000), 0)
        
        
    def testAmdahlModelLinear(self):
        points = [DataPoint(cpuCount=1, cpu=1, time=1000, probSize=35), 
                  DataPoint(cpuCount=2, cpu=1, time=600, probSize=35), 
                  DataPoint(cpuCount=2, cpu=1, time=1200, probSize=70)]
        
        model = AmdahlsCompModel(dataPoints=points, scaleFunction='linear')
        
        self.assertEquals(1000, model.modelFunc(probSize=35, cpu=1, count=1))
        self.assertEquals(600, model.modelFunc(probSize=35, cpu=1, count=2))
        self.assertAlmostEquals(400, model.modelFunc(probSize=35, cpu=1, count=4))
        self.assertAlmostEquals(300, model.modelFunc(probSize=35, cpu=1, count=8))
        
        self.assertAlmostEquals(1200, model.modelFunc(probSize=70, cpu=1, count=2))
        self.assertAlmostEquals(2400, model.modelFunc(probSize=140, cpu=1, count=2))
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
        
        
    def testAmdahlModelLinearVariousCpu(self):
        points = [DataPoint(cpuCount=1, cpu=1, time=1000, probSize=35), 
                  DataPoint(cpuCount=2, cpu=2, time=300, probSize=35), 
                  DataPoint(cpuCount=2, cpu=1, time=1200, probSize=70)]
        
        model = AmdahlsCompModel(dataPoints=points, scaleFunction='linear')
        
        self.assertEquals(1000, model.modelFunc(probSize=35, cpu=1, count=1))
        self.assertEquals(600, model.modelFunc(probSize=35, cpu=1, count=2))
        self.assertAlmostEquals(400, model.modelFunc(probSize=35, cpu=1, count=4))
        self.assertAlmostEquals(300, model.modelFunc(probSize=35, cpu=1, count=8))
        
        self.assertAlmostEquals(1200, model.modelFunc(probSize=70, cpu=1, count=2))
        self.assertAlmostEquals(2400, model.modelFunc(probSize=140, cpu=1, count=2))
        
    def testAmdahlModelLinearVariousCpu2(self):
        points = [DataPoint(cpuCount=1, cpu=1, time=1000, probSize=35), 
                  DataPoint(cpuCount=2, cpu=2, time=300, probSize=35), 
                  DataPoint(cpuCount=2, cpu=1, time=1200, probSize=70)]
        
        model = AmdahlsCompModel(dataPoints=points, scaleFunction='linear')
        
        self.assertEquals(1000, model.modelFunc(probSize=35, cpu=1, count=1))
        self.assertEquals(600, model.modelFunc(probSize=35, cpu=1, count=2))
        self.assertAlmostEquals(400, model.modelFunc(probSize=35, cpu=1, count=4))
        self.assertAlmostEquals(300, model.modelFunc(probSize=35, cpu=1, count=8))
        
        self.assertAlmostEquals(1200, model.modelFunc(probSize=70, cpu=1, count=2))
        self.assertAlmostEquals(1200, model.modelFunc(probSize=140, cpu=2, count=2))
        
    def testAmdahlModelLog(self):
        points = [DataPoint(cpuCount=1, cpu=1, time=1000, probSize=35), 
                  DataPoint(cpuCount=2, cpu=1, time=600, probSize=35), 
                  DataPoint(cpuCount=2, cpu=1, time=1200, probSize=70)]
        
        model = AmdahlsCompModel(dataPoints=points, scaleFunction='log')
        
        #self.assertEquals(1000, model.modelFunc(probSize=35, cpu=1, count=1))
        #self.assertEquals(600, model.modelFunc(probSize=35, cpu=1, count=2))
        #self.assertEquals(1200, model.modelFunc(probSize=70, cpu=1, count=2))
        
    def testPolyCompModel2Simple(self):
        points = [DataPoint(cpuCount=1, cpu=1, time=1000, probSize=35), 
                  DataPoint(cpuCount=2, cpu=2, time=300, probSize=35), 
                  DataPoint(cpuCount=1, cpu=1, time=2000, probSize=70)]
        
        model = PolyCompModel2(dataPoints=points, scaleFunction='linear')
        
        self.assertAlmostEquals(1.66666666, model.speedUpFunc(2))
        
        self.assertAlmostEquals(1000, model.time_1_func(35))
        self.assertAlmostEquals(2000, model.time_1_func(70))
        
        #self.assertEquals(1000, model.modelFunc(probSize=35, cpu=1, count=1))
        #self.assertEquals(600, model.modelFunc(probSize=35, cpu=1, count=2))
        #self.assertAlmostEquals(400, model.modelFunc(probSize=35, cpu=1, count=4))
        #self.assertAlmostEquals(300, model.modelFunc(probSize=35, cpu=1, count=8))
        
        #self.assertAlmostEquals(1200, model.modelFunc(probSize=70, cpu=1, count=2))
        #self.assertAlmostEquals(1200, model.modelFunc(probSize=140, cpu=2, count=2))
        
    def testPolyCompModel2Simple2(self):
        points = [DataPoint(cpuCount=1, cpu=1, time=1100, probSize=35), 
                  DataPoint(cpuCount=2, cpu=2, time=300, probSize=35), 
                  DataPoint(cpuCount=1, cpu=1, time=2000, probSize=70),
                  DataPoint(cpuCount=1, cpu=1, time=3100, probSize=105)]
        
        model = PolyCompModel2(dataPoints=points, scaleFunction='linear')
        
        ''' Assert speed up func matches observation '''
        self.assertAlmostEquals(1.83333333, model.speedUpFunc(2))
        
        ''' Assert speed up func matches expectation'''
        self.assertAlmostEquals(2.666666666, model.speedUpFunc(3))
        
        
        
        ''' Test that the single proc line fits between the observations '''
        self.assertAlmostEquals(1066.66666666, model.time_1_func(35))
        self.assertAlmostEquals(2066.66666666, model.time_1_func(70))
        self.assertAlmostEquals(3066.66666666, model.time_1_func(105))
        
        self.assertAlmostEquals(400.0000, model.modelFunc(probSize=35, cpu=1, count=3))
        
    def testPolyCompModel2Cost(self):
        points = [DataPoint(cpuCount=1, cpu=1, time=100, probSize=35), 
                  DataPoint(cpuCount=2, cpu=2, time=30, probSize=35), 
                  DataPoint(cpuCount=1, cpu=1, time=200, probSize=70)]
        
        model = PolyCompModel2(dataPoints=points, scaleFunction='linear')
        
        cloud = EC2Cloud(None, 
                        name="Ec2", 
                        serviceURL="",
                        storageURL="",
                        botoModule=None,
                        ec2Cert="",
                        instanceTypes=[
                                       InstanceType('m1.small', 2, 1, 256, 2, None, 1.0),
                                       #InstanceType('m1.large', 2, 2, 1792, 15, None, 2.0),
                                       #InstanceType('m1.xlarge', 2, 1, 1792, 20, None, 3.0),
                                       #InstanceType('c1.medium', 2, 2, 512, 5, None, 2.0),
                                       #InstanceType('c1.xlarge', 2, 4, 1792, 20, None, 3.0)
                                       ]
                                       )
        
        costModel = model.costModel(cloud)
        self.assertNotEquals(None, costModel)
        
        ''' Small problems will go for single cheapest instance'''
        self.assertEquals(1.0, costModel(1))
        self.assertEquals(1.0, costModel(0.1))
        
        self.assertAlmostEquals(2642.8571428574, model.modelFunc(probSize=25 * 37 * 2, cpu=2, count=1 ))
        
        ''' Should run on single m1.small for  1hr< time <2hr'''
        self.assertEquals(2.0, costModel(35 * 37 * 2))
        self.assertEquals(2.0, costModel(35 * 38 * 2))
        

if __name__ == "__main__":
    #import sys;sys.argv = ['', 'Test.testName']
    unittest.main()

    
from numpy import true_divide, array
from scipy.optimize import leastsq

import math
import sys

class DataPoint(object):
    
    def __init__(self, deployment):
        self.machineCount = reduce(lambda x,y: x+y, [r.count * r.instanceType.cpuCount for r in deployment.roles])
        self.cpu = deployment.roles[0].instanceType.cpu
        self.time = deployment.roleRunTime()
        self.probSize = deployment.problemSize
    
class CompModel(object):

    def __init__(self):
        pass
        
        
    def costModel(self, cloud):
        '''
        Return a cost model function based on the runtime model generated and the cloud provided.
        The cost model assumes a homogeneous cluster. 
        The cost model returns a BestDeployment object for a given problem size.
        '''
        def model(problemSize):
            
            bestCost = sys.maxint
            bestType = None
            bestTime = None
            bestCount = None
            
            for instanceType in cloud.instanceTypes:
                for c in range(1, 100):
                    t = self.modelFunc(problemSize, instanceType.cpu, c*instanceType.cpuCount)
                    hours = math.ceil(true_divide(t, 60))
                    cost = instanceType.costPerHour * c * hours
                    if cost < bestCost:
                        bestTime = t
                        bestType = instanceType
                        bestCount = c
                        bestCost = cost
                        
            return (bestCost, bestTime, bestType, bestCount)
        
        return model
 
class AmdahlsCompModel(CompModel):       
        
    def __init__(self, deploymentTemplate):
        
        CompModel.__init__(self)
        
        self.deploymentTemplate = deploymentTemplate
        self.dataPoints = []
        self.__collectDataPoints()
        self.__bestFit()
        
    def __collectDataPoints(self):
        
        for dep in self.deploymentTemplate.deployments:
            self.dataPoints.append(DataPoint(dep))
    
    def __bestFit(self):
        
        #Get T(1)
        t1s = [(d.time, d.probSize) for d in self.dataPoints if d.machineCount == 1]
                
                
        for t1,size in t1s:
            txs = [(d.time, d.machineCount) for d in self.dataPoints if d.probSize == size and 
                    d.machineCount == 1]
            
            
            # p = (speedUp(n) - 1) / speedUp(2)
            
            p = [tx[1] * (t1 - tx[0] - 1) / (t1 - tx[0]) for tx in txs ]
                
       
        
        
        def runTime(v, probSize, machineCpu, count):
            x = true_divide(probSize , machineCpu * count)
            return v[0] + v[1] * x #+ v[2] * (x**2) #+ v[3] * (x**3)
        
        # Error Function
        ef = lambda v, probSize, cpu, cnt, t: (runTime(v, probSize, cpu, cnt)-t)
        
        realProbSize = array([d.probSize for d in self.dataPoints])
        cpu = array([d.cpu for d in self.dataPoints])
        counts = array([d.machineCount for d in self.dataPoints])
        times = array([d.time for d in self.dataPoints])
        
        v0 = [1,1,1]
        testVal = ((v0[:4],)+(realProbSize, cpu, counts, times))
        v, success = leastsq(ef, v0, args=(realProbSize, cpu, counts, times), maxfev=10000)
        
        self.modelFunc = lambda ps, cpu, count: runTime(v, ps, cpu, count)
    
class PolyCompModel(CompModel):
    
    def __init__(self, deploymentTemplate):
        
        CompModel.__init__(self)
        
        self.deploymentTemplate = deploymentTemplate
        self.dataPoints = []
        self.__collectDataPoints()
        
        self.__bestFit()
        
    def __collectDataPoints(self):
        
        for dep in self.deploymentTemplate.deployments:
            self.dataPoints.append(DataPoint(dep))
    
    def __bestFit(self):
        
        def runTime(v, probSize, machineCpu, count):
            x = true_divide(probSize , machineCpu * count)
            return v[0] + v[1] * x #+ v[2] * (x**2) #+ v[3] * (x**3)
        
        # Error Function
        ef = lambda v, probSize, cpu, cnt, t: (runTime(v, probSize, cpu, cnt)-t)
        
        realProbSize = array([d.probSize for d in self.dataPoints])
        cpu = array([d.cpu for d in self.dataPoints])
        counts = array([d.machineCount for d in self.dataPoints])
        times = array([d.time for d in self.dataPoints])
        
        v0 = [1,1,1]
        testVal = ((v0[:4],)+(realProbSize, cpu, counts, times))
        v, success = leastsq(ef, v0, args=(realProbSize, cpu, counts, times), maxfev=10000)
        
        self.modelFunc = lambda ps, cpu, count: runTime(v, ps, cpu, count)
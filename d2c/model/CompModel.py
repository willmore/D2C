from numpy import true_divide, array
from scipy.optimize import leastsq
import numpy

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

# http://en.wikipedia.org/wiki/Amdahl's_law#Parallelization
def pEstimated(speedUp, numberProcessors):
    return (1 / speedUp - 1) / (1 / numberProcessors - 1)

def speedUp(n, p):
    '''
    Return speedup for n processors given parallelizable percentage p.
    '''
    return 1 / ((p / n) + (1 - p))

def runTime(t1, p, n):
    '''
    Runtime of a fixed problem size, given base runtime t1 and speedup(n, p)
    '''
    return t1 / speedUp(n, p)

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
                
        ps = []        
        for t1,size in t1s:
            txs = [(d.time, d.machineCount) for d in self.dataPoints if d.probSize == size and 
                    d.machineCount != 1]
            

            # SpeedUp(x) = t1 / t(x)             
            ps.append([pEstimated(true_divide(t1, tx), np) for tx,np in txs ])
           
        # Lets just take the mean for now 
        p = numpy.mean(ps)
        
        
        sizeFactor = lambda probSize, baseRunTime : baseRunTime + (t1s[0][0] / t1s[0][1]) * probSize
        
        self.modelFunc = lambda probSize, cpu, count: sizeFactor(probSize, runTime(t1, p, count))
    
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
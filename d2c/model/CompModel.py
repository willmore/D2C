from numpy import true_divide, array
from scipy.optimize import leastsq
import numpy

import math
import sys

class DataPoint(object):
    
    def __init__(self, deployment=None, cpuCount=None, 
                 cpu=None, time=None, probSize=None):
        
        if deployment is not None and \
            (cpuCount is not None or cpu is not None or time is not None or probSize is not None):
            raise Exception("Illegal arguments")
        
        if deployment is not None:
            self.machineCount = reduce(lambda x,y: x+y, [r.count * r.instanceType.cpuCount for r in deployment.roles])
            self.cpu = deployment.roles[0].instanceType.cpu
            self.time = deployment.roleRunTime()
            self.probSize = deployment.problemSize
        else:
            self.machineCount = cpuCount
            self.cpu = cpu
            self.time = time
            self.probSize = probSize
        

def toDataPoints(deploymentTemplate):
    '''
    Map a DeploymentTemplate to a list of DataPoints
    '''
    return [DataPoint(d) for d in deploymentTemplate.deployments]
    
class CompModel(object):

    def __init__(self, deploymentTemplate=None, dataPoints=None):
        if deploymentTemplate is not None and \
            dataPoints is not None:
            raise Exception("Only one of deploymentTemplate and dataPoints may be set (not None)")
        
        self.dataPoints = toDataPoints(deploymentTemplate) if deploymentTemplate is not None else dataPoints
        
        
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
    assert numberProcessors > 1
    assert speedUp > 0
    assert numberProcessors == int(numberProcessors)
    return (true_divide(1, speedUp) - 1) / (true_divide(1, numberProcessors) - 1)

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
        
    def __init__(self, deploymentTemplate=None, dataPoints=None, scaleFunction='linear'):
        
        CompModel.__init__(self, deploymentTemplate, dataPoints)
        
        self.__generateModel(scaleFunction)
    
    def __generateModel(self, scaleFunction='linear'):
        '''
        The performance model is based on two assumptions:
            1. Runtime scales non-linearly using Amdahl's law as
               the problem size remains fixed and the number of processors
               increases.
            2. The runtime scales linearly as the problem size increases and the
               number of processors remains fixed.
        '''
        
        '''
        Calculate parameters for scaling based on Amdahl's Law
        '''
        
        '''
        The first step is to get T(1) which is the runtime of calculating problem size N
        of one processor. 
        '''
        t1s = [(d.time, d.probSize) for d in self.dataPoints if d.machineCount == 1]
           
        '''
        If we don't have a T(1), bail.
        '''   
        if len(t1s) == 0:
            raise Exception("Unable to create model. \
                             Require at least one data point where the CPU count is 1.")        
                
        '''
        Next we find values of T(x) where the number of processors is x > 1 and the problem
        size is N (same size as T(1)).
        '''
        
        ''' ps is a mapping from problem size N to a list of (x, runTime) tuples '''
        ps = {}        
        for t1,N in t1s:
            ps[N] = [(d.time, d.machineCount) for d in self.dataPoints if d.probSize == N and 
                    d.machineCount != 1]
            
        '''
        Now we generate estimates for P, the percentage of the program measured that is parallelizable,
        based on (N, T(1), T(x)) tuples
        
        p can be calculated given speed increase (x) = T(1) / T(x) and x
        '''    
        pEstimates = []
        for t1, t1_N in t1s:
            for (tx, x) in ps[t1_N]:
                pEstimates.append(pEstimated(true_divide(t1, tx), x))
           
        # Lets just take the mean for now 
        p = numpy.mean(pEstimates)
        
        '''
        Now to calculate scaling of problem size.
        Find all pairs where cpu count is same but prob size increases
        '''
        if scaleFunction == 'linear':
            sf = self.__generateLinearScaleFunction()[0]
        elif scaleFunction == 'log':
            sf = self.__generateLogScaleFunction()[0]
        
        
        self.modelFunc = lambda probSize, cpu, count: sf(probSize, (t1s[0][1],runTime(t1, p, count)))
    
    def __generateLogScaleFunction(self):
        
        funcs = []
        
        for dp1 in self.dataPoints:
            
            points = [dp2 for dp2 in self.dataPoints 
                           if dp2 is not dp1 and dp2.machineCount == dp1.machineCount]
            points.append(dp1)
            
            if len(points) < 1:
                continue
            
            points = [(dp.probSize, dp.time) for dp in points]
            
            def runTime(v, probSize):
                return v[0] * numpy.log(probSize)
            
            ef = lambda v, probSize, t: (runTime(v, probSize)-t)
            
            v0 = [1]
            v, success = leastsq(ef, v0, 
                                 args=(array([x for x,y in points]),
                                       array([y for x,y in points])), 
                                 maxfev=10000)
            
            funcs.append(lambda ps, dp: runTime(v, ps) + (dp[1] - runTime(v, dp[0])))
        
        return funcs
        
    def __generateLinearScaleFunction(self):
        
        #slopeIntersectPairs = []
        slopes = []
        for dp1 in self.dataPoints:
            slopes.extend([true_divide((dp1.time - dp2.time), (dp1.probSize - dp2.probSize)) 
                           for dp2 in self.dataPoints 
                           if dp2 is not dp1 and dp2.machineCount == dp1.machineCount])

        #dp = (x,y)
        return [lambda probSize, dp: probSize * slope + (dp[1] - slope * dp[0])
                for slope in slopes]
        
    
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
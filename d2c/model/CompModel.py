from numpy import true_divide, array, ceil, minimum, clip, mean, subtract, add, where
from scipy.optimize import leastsq
import numpy
import types

from .Deployment import DeploymentState

#import math
import sys
from abc import ABCMeta, abstractmethod


class ProbSizeScaleFunctionGenerator:
    
    __metaclass__ = ABCMeta
    
    def __init__(self):
        pass
    
    @abstractmethod
    def generate(self, dataPoints):
        pass
   
class LogProbSizeScaleFunctionGenerator(ProbSizeScaleFunctionGenerator):
    
    def __init__(self):
        ProbSizeScaleFunctionGenerator.__init__(self)
        
    def generate(self, dataPoints):
    
        funcs = []
        
        for dp1 in dataPoints:
            
            points = [dp2 for dp2 in dataPoints 
                           if dp2 is not dp1 and dp2.machineCount == dp1.machineCount]
            points.append(dp1)
            
            if len(points) < 1:
                continue
            
            points = [(dp.probSize, dp.normalizedTime) for dp in points]
            
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
        
class NLogProbSizeScaleFunctionGenerator(ProbSizeScaleFunctionGenerator):
    
    def __init__(self):
        ProbSizeScaleFunctionGenerator.__init__(self)
        
    def generate(self, dataPoints): 
                
        funcs = []
        
        for dp1 in dataPoints:
            
            points = [dp2 for dp2 in dataPoints 
                           if dp2 is not dp1 and dp2.machineCount == dp1.machineCount]
            points.append(dp1)
            
            if len(points) < 2:
                continue
            
            points = [(dp.probSize, dp.normalizedTime) for dp in points]
            
            def runTime(v, probSize):
                return v[0] + v[1] * probSize * numpy.log(probSize)
            
            ef = lambda v, probSize, t: (runTime(v, probSize)-t)
            
            v0 = [1,1]
            v, success = leastsq(ef, v0, 
                                 args=(array([x for x,y in points]),
                                       array([y for x,y in points])), 
                                 maxfev=10000)
            
            funcs.append(lambda ps, dp: runTime(v, ps) + (dp[1] - runTime(v, dp[0])))
        
        return funcs


class LinearProbSizeScaleFunctionGenerator(ProbSizeScaleFunctionGenerator):
    
    def __init__(self):
        ProbSizeScaleFunctionGenerator.__init__(self)
        
    def generate(self, dataPoints):
        
        def runTime(v, probSize):
            '''
            v function coefficients
            probSize size of the problem
            '''
            return v[0] + v[1] * probSize
        
        ef = lambda v, probSize, t: (runTime(v, probSize)-t)
        
        funcs = []
        
        for dp1 in dataPoints:
            
            points = [dp2 for dp2 in dataPoints 
                           if dp2 is not dp1 and dp2.machineCount == dp1.machineCount]
            
            points.append(dp1)
        
            if len(points) < 2:
                continue
            
            points = [(dp.probSize, dp.normalizedTime) for dp in points]
            
            v0 = [1,1]
            v, success = leastsq(ef, v0, 
                                 args=(array([x for x,y in points]),
                                       array([y for x,y in points])), 
                                 maxfev=10000)
            
            funcs.append(lambda ps, dp: runTime(v, ps) + (dp[1] - runTime(v, dp[0])))
        
        return funcs

class DataPoint(object):
    
    __metaclass__ = ABCMeta
    

class SimpleDataPoint(DataPoint):

    def __init__(self, deployment=None, cpuCount=None, 
                 cpu=None, time=None, probSize=None, totalMemory=None):
        '''
        totalMemory = peak memory used by the application across the cluster
        '''
        
        DataPoint.__init__(self)  
        
        self.machineCount = cpuCount
        self.cpu = cpu
        self.time = time
        self.probSize = probSize
        self.normalizedTime = self.cpu * self.time
        self.totalMemory = totalMemory

class DeploymentDataPoint(DataPoint):
    
    def __init__(self, deployment):
        
        DataPoint.__init__(self)
        
        self.machineCount = reduce(lambda x,y: x+y, [r.count * r.instanceType.cpuCount for r in deployment.roles])
        self.cpu = deployment.roles[0].instanceType.cpu
        self.time = deployment.roleRunTime()
        self.probSize = deployment.problemSize
        self.totalMemory = deployment.getMaxMemory()
        self.normalizedTime = self.cpu * self.time

def toDataPoints(deploymentTemplate):
    '''
    Map a DeploymentTemplate to a list of DataPoints
    '''
    return [DeploymentDataPoint(d) for d in deploymentTemplate.deployments if d.state == DeploymentState.COMPLETED]
    
class CompModel:

    __metaclass__ = ABCMeta

    def __init__(self, deploymentTemplate=None, dataPoints=None):
        if deploymentTemplate is not None and \
            dataPoints is not None:
            raise Exception("Only one of deploymentTemplate and dataPoints may be set (not None)")
        
        self.dataPoints = toDataPoints(deploymentTemplate) if deploymentTemplate is not None else dataPoints
        self.modelFunc = None
        self.memoryModel = None
        self.__generateMemoryModel()
        
    def __generateMemoryModel(self):
        '''
        Create self.memoryFunc which has as input N problem size and output M max memory required to solve
        '''    
        def memory_v(v, N):
            return v[0] + v[1]*N
        
        ef = lambda v, N, M: (memory_v(v, N)-M)
        v0 = [1.,1.]
        Ns = array([d.probSize for d in self.dataPoints])
        Ms = array([d.totalMemory for d in self.dataPoints])
        Vs, success = leastsq(ef, v0, args=(Ns, Ms), maxfev=10000)
        
        def memoryModel(N):
            return memory_v(Vs, N)
        
        self.memoryModel = memoryModel
        
        
    def costModel(self, cloud):
        '''
        Return a cost model function based on the runtime model generated and the cloud provided.
        The cost model assumes a homogeneous cluster. 
        The cost model returns a BestDeployment object for a given problem size.
        '''
        
        print "Creating cost func for cloud ", cloud.name
        
        def model(problemSize):
            try:
                bestCost = array([sys.maxint for _ in problemSize])
            except:
                bestCost = sys.maxint
                
            for instanceType in cloud.instanceTypes:
                for c in xrange(1, 64):
                    ''' Filter for memory requirements '''
                    fits = self.memoryModel(problemSize) > (c * instanceType.memory)
                    
                    t = self.modelFunc(problemSize, instanceType.cpu, c*instanceType.cpuCount)
                    hours = ceil(true_divide(t, 3600.0))
                    cost = instanceType.costPerHour * c * hours
                    
                    bestCost = where(fits, minimum(bestCost, cost), bestCost)
                    #bestCost = minimum(bestCost, cost)

            print "Best cost", bestCost
            return bestCost
            
        return model
        
    def modelSumOfSquares(self, dataPoints):
        '''
        Given a set of real data points, return the sum of squares of using 
        difference of observed time and expected time.
        '''
        def sumOfSquares(dps):
            return reduce(sum, [(self.modelFunc(dp.probSize, dp.cpu, dp.machineCount) - dp.time)**2 for dp in dps])
        
        return sumOfSquares(dataPoints)
    
    probSizeScalGens = {
                        'linear' : LinearProbSizeScaleFunctionGenerator(),
                        'log' : LogProbSizeScaleFunctionGenerator(),
                        'nlog' : NLogProbSizeScaleFunctionGenerator()
                        }
    
    def generateProbSizeScaleFunction(self, scaleFunction):
        return self.probSizeScalGens[scaleFunction].generate(self.dataPoints)

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

def sum(v1, v2):
    return v1 + v2

def sumOfSquares(f, dps):
    return reduce(sum, [(f(dp.probSize, dp.machineCount) - dp.normalizedTime)**2 for dp in dps])

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
        t1s = [(d.normalizedTime, d.probSize) for d in self.dataPoints if d.machineCount == 1]
           
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
            ps[N] = [(d.normalizedTime, d.machineCount) for d in self.dataPoints if d.probSize == N and 
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
        sfs = self.generateProbSizeScaleFunction(scaleFunction)    
        
        functions = [lambda probSize, count: sf(probSize, (t1s[0][1],runTime(t1, p, count)))
                     for sf in sfs]
        
        bestSf = reduce(lambda f1, f2 : f1 if sumOfSquares(f1, self.dataPoints) < sumOfSquares(f2, self.dataPoints) 
                                            else f2, 
                        functions)
        
        ''' The real function corrects for cpu speed '''
        self.modelFunc = lambda probSize, cpu, count: bestSf(probSize, count) / cpu
      
    
class PolyCompModel(CompModel):
    
    def __init__(self, deploymentTemplate=None, dataPoints=None, scaleFunction='linear'):
        
        CompModel.__init__(self, deploymentTemplate, dataPoints)
        
        #self.__generateModel(scaleFunction)
        self.__generateModel()
        
    def __generateModel(self):
        
        def runTime(v, probSize, count):
            x = true_divide(probSize , count)
            return v[0] + v[1] * x  #+ v[3] * (x**3)
        
        # Error Function
        ef = lambda v, probSize, cpu, cnt, t: (runTime(v, probSize, cnt)-t)
        
        realProbSize = array([d.probSize for d in self.dataPoints])
        cpu = array([d.cpu for d in self.dataPoints])
        counts = array([d.machineCount for d in self.dataPoints])
        times = array([d.normalizedTime for d in self.dataPoints])
        
        v0 = [1,1,1]
        v, success = leastsq(ef, v0, args=(realProbSize, cpu, counts, times), maxfev=10000)
        
        self.modelFunc = lambda ps, cpu, count: runTime(v, ps, count) / cpu
        
        
class GustafsonCompModel(CompModel):
    
    def __init__(self, deploymentTemplate=None, dataPoints=None, scaleFunction='linear'):
        
        CompModel.__init__(self, deploymentTemplate, dataPoints)
        
        self.__generateModel(scaleFunction)
        #self.__generateModel()
        
    def __generateModel(self, scaleFunction):
        
        '''
        Generate a function that calculates runtime based on 
        problem size and number of processors
        '''
        
        ''' s is percentage of program that is serialized 
        speedup(n) = n + (1-n)s
        s = (speedup(n) - n) / (1 - n)
        Find all known speedup(n) where problem size is constant
        speedup(n) also equals T(1) / T(n). We should have T(1) and T(n)
        '''
        t1s = [(d.normalizedTime, d.probSize) for d in self.dataPoints if d.machineCount == 1]
        
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
        
        ''' ss are possible values of s '''
        speed_ups = []        
        for t1,ps in t1s:
            for d in [d for d in self.dataPoints if d.probSize == ps and d.machineCount != 1]:
                speed_ups.append((d.machineCount, true_divide(t1, d.normalizedTime)))
                #s = true_divide(speedup_n - n, 1 - n)
                #ss.append(s)
        
        def speed_up_v(v, n): 
            return n + (1 - n) * v[0]
        
        ef = lambda v, n, real_su: (speed_up_v(v, n)- real_su)
        v0 = [1]
        v_su, success = leastsq(ef, v0, args=(array([n for n,su in speed_ups]), array([su for n,su in speed_ups])), maxfev=100000)
        
        if not (success in (1,2,3,4)):
            raise Exception("leastsq error code %d" % success)
        
        def speedup(n):
            su = speed_up_v(v_su, n)
            return su
        
        self.speedUpFunc = speedup
        
        def time_1_v(v, probSize):
            '''Time for running probSize on one cpu'''
            return v[0] + v[1]*probSize
        
        ef = lambda v, probSize, t: (time_1_v(v, probSize)-t)
        v0 = [100,1]
        v_t1, success = leastsq(ef, v0, args=(array([ps for t,ps in t1s]), array([t for t,ps in t1s])), maxfev=10000)
        
        def time_1(probSize):
            '''Time for running probSize on one cpu'''
            return time_1_v(v_t1, probSize)
        
        self.time_1_func = time_1
        
        def modelFunc(probSize, cpu, count):
            t1 = time_1(probSize)
            su = speedup(count)
            return (t1 / su) / cpu
        
        #self.modelFunc = lambda probSize, cpu, count: true_divide(time_1(probSize), speedup(n)) / cpu
        self.modelFunc = modelFunc
        
        
class GustafsonCompModel2(CompModel):
    
    def __init__(self, deploymentTemplate=None, dataPoints=None, scaleFunction='linear'):
        
        CompModel.__init__(self, deploymentTemplate, dataPoints)
        
        self.__generateModel(scaleFunction)
        #self.__generateModel()
        
    def __generateModel(self, scaleFunction):
        
        '''
        Generate a function that calculates runtime based on 
        problem size and number of processors
        '''
        
        ''' 
        s is percentage of program that is serialized 
        speedup(n) = n + (1-n)s
        s = (speedup(n) - n) / (1 - n)
        Find all known speedup(n) where problem size is constant
        speedup(n) also equals T(1) / T(n). We should have T(1) and T(n)
        '''
        t1s = [(d.normalizedTime, d.probSize) for d in self.dataPoints if d.machineCount == 1]
        
        '''
        If we don't have a T(1), bail.
        '''   
        if len(t1s) == 0:
            raise Exception("Unable to create model. \
                             Require at least one data point where the CPU count is 1.")        
        serialFracFunc = estimate_serial_frac(self.dataPoints)
        self.speedUpFunc = lambda cpuCount, probSize: scaled_speedup(cpuCount, probSize, serialFracFunc)
        
        def time_1_v(v, probSize):
            '''Time for running probSize on one cpu'''
            return v[0] + v[1]*probSize
        
        ef = lambda v, probSize, t: (time_1_v(v, probSize)-t)
        v0 = [1,1]
        v_t1, success = leastsq(ef, v0, args=(array([ps for t,ps in t1s]), array([t for t,ps in t1s])), maxfev=10000)
        
        def time_1(probSize):
            '''Time for running probSize on one cpu'''
            return time_1_v(v_t1, probSize)
        
        self.time_1_func = time_1
        
        def modelFunc(probSize, cpu, count):
            t1 = time_1(probSize)
            su = self.speedUpFunc(count, probSize)
            return (t1 / su) / cpu
        
        #self.modelFunc = lambda probSize, cpu, count: true_divide(time_1(probSize), speedup(n)) / cpu
        self.modelFunc = modelFunc
        
        
class PolyCompModel3(CompModel):
    
    def __init__(self, deploymentTemplate=None, dataPoints=None, scaleFunction='linear'):
        
        CompModel.__init__(self, deploymentTemplate, dataPoints)
        
        self.__generateModel(scaleFunction)
        #self.__generateModel()
        
    def __generateModel(self, scaleFunction):
          
        def model_v(v, probSize, cpu, count):
            return ( (v[0] + probSize * v[1]) / (count - (count - 1) * (1 / (v[2] + v[3]*probSize))) ) / cpu
        
        ef = lambda v, probSize, cpu, count, time: (model_v(v, probSize, cpu, count)- time)
        v0 = [1.,1.,1.,1.]
        
        probSizes = array([p.probSize for p in self.dataPoints])
        cpus = array([p.cpu for p in self.dataPoints])
        counts = array([p.machineCount for p in self.dataPoints])
        times = array([p.normalizedTime for p in self.dataPoints])
        
        v, success = leastsq(ef, v0, args=(probSizes, cpus, counts, times), maxfev=10000)
        if not success in (1,2,3,4):
            raise Exception("No solution found")
        #print "Message = ", mesg
        print "Vs = ", v
        def model(probSize, cpu, count):
            return model_v(v, probSize, cpu, count)
        
        self.modelFunc = model

def scaled_speedup(N, probSize, serial_func):
    '''
    http://software.intel.com/en-us/articles/amdahls-law-gustafsons-trend-and-the-performance-limits-of-parallel-applications/
    '''
    S = serial_func(probSize)
    
    def overhead(N):
        return .6*N
    
    return S + N * (1 - S) - overhead(N)

def estimate_serial_frac(dps):
    '''
    Given set of data points for same experiment, generate a function
    which gives the serial frac vs problem size
    '''
    
    ''' organize by same problem size '''
    probSizeDPMap = {} # probSize (int) => list of dps
    for dp in dps:
        if not dp.probSize in probSizeDPMap:
            probSizeDPMap[dp.probSize] = []
        probSizeDPMap[dp.probSize].append(dp)
    
    probSizes = []
    serialTimes = []
        
    for probSize, dps in probSizeDPMap.iteritems():
        
        if len(dps) < 2:
            continue

        singleCPUDataPoints = [dp for dp in dps if dp.machineCount == 1]
        multipleCPUDataPoints = [dp for dp in dps if dp.machineCount > 1]
        
        if len(singleCPUDataPoints) == 0:
            continue
        if len(multipleCPUDataPoints) == 0:
            continue
        
        serializedTimes = []
        for dp1 in singleCPUDataPoints:
            for dp2 in  multipleCPUDataPoints:
                serializedTimes.append(serial_fraction(dp1, dp2))
                
        ''' 
        TODO
        Don't know yet how to treat range of serialized time for single prob size
        Taking the mean for now...
        '''
        probSize = singleCPUDataPoints[0].probSize
        probSizes.append(probSize)
        serialTimes.append(mean(serializedTimes))
    
    '''
    Next we fit a function to estimated serialized times and problem sizes.
    '''
        
    def serial_frac_v(v, ps):
        return v[0] / ps
    
    ef = lambda v, probSize, serial: (serial_frac_v(v, probSize) - serial)
    
    v0 = [1.]
    vs, success = leastsq(ef, v0, args=(array(probSizes), array(serialTimes)), maxfev=10000)
    
    return lambda probSize: serial_frac_v(vs, probSize)
    

def serial_fraction(dp1, dp2):
    
    '''
    Based on Gustafon's, find the relative serial time spent based on two data points
    with equal problem size, but different number of processors.
    '''
    assert dp1.machineCount == 1
    assert dp1.probSize == dp2.probSize, "ProbSize's must be equal"
    assert dp1.normalizedTime != dp2.normalizedTime, "Times must not be equal"
    
    return true_divide(serial_time(dp1, dp2), dp2.normalizedTime)

def serial_time(dp1, dp2):
    '''
    Based on Gustafon's, find the serial time spent based on two data points
    with equal problem size, but different number of processors.
    '''
    assert dp1.machineCount == 1
    assert dp1.probSize == dp2.probSize, "ProbSize's must be equal"
    assert dp1.normalizedTime != dp2.normalizedTime, "Times must not be equal"
    
    '''
    serial time s
    parallel time p
    total time t
    number of cpus n
    
    t1 = s' + n*p'
    t2 = s' + p'
    p' = (t1 - t2) / (n - 1)
    let t_diff = (t1 - t2)
    '''
    t_diff = dp1.normalizedTime - dp2.normalizedTime
    p = t_diff / (dp2.machineCount - 1.)
    s = dp2.normalizedTime - p
    
    return s   
        
from numpy import true_divide, array
from scipy.optimize import leastsq

class DataPoint(object):
    
    def __init__(self, deployment):
        self.machineCount = reduce(lambda x,y: x+y, [r.count for r in deployment.roles])
        self.cpu = deployment.roles[0].instanceType.cpu
        self.time = deployment.roleRunTime()
        self.probSize = deployment.problemSize
    
class CompModel(object):

    def __init__(self, dataPoints):
        self.dataPoints = dataPoints
        
class PolyCompModel(object):
    
    def __init__(self, deploymentTemplate):
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
            return v[0] + v[1] * x + v[2] * (x**2) #+ v[3] * (x**3)
        
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
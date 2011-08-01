from pylab import *
from numpy import *
import numpy
from scipy.optimize import leastsq


machineTypes = ({"cpu":3}, {"cpu":4.5})
#machineTypes = ({"cpu":3},)

def minarray(a1, a2):
    #print "a1 = %s a2 = %s" % (str(a1), str(a2))
    out = []
    for n in range(0,len(a1)):
        out.append(a1[n] if a1[n] < a2[n] else a2[n])
        
    return out

def runTime(v, probSize, machineCpu, count):
    
    x = true_divide(probSize , machineCpu * count)
    
    t = v[0] + v[1] * x + v[2] * (x**2) #+ v[3] * (x**3)
    return t



times = array([5, 100, 110, 170, 250, 200, 120, 400])
realProbSize = array([0, 100, 100, 200, 300, 400, 400,1600])
cpu = array([1500, 1500, 1500, 1500, 1500, 2500, 2500,2500])
counts = array([1,  1,   1, 1,    1,    1, 2,4])

## Parametric function: 'v' is the parameter vector
#fp = lambda v, cpu, probSize: v[0] + v[1](probSize / cpu)

#def fp(v, probSize, cpu):round
#    return v[0] + v[1]*(probSize / cpu)

## Noisy function (used to generate data to fit)
#v_real = [1.5, 2]
#fn = lambda x: fp(v_real, x)

#def fp(v, probSize):
#    return runTime(v, probSize, machineTypes[0], 2)
    #return runTime2(v, probSize)

## Error function
e = lambda v, probSize, cpu, cnt, t: (runTime(v,probSize,cpu, cnt)-t)

## Generating noisy data to fit
n = 5
xmin = 1
xmax = numpy.max(realProbSize) * 2


## Initial parameter value
v0 = [1,1,10,1,1]

## Fitting
v, success = leastsq(e, v0, args=(realProbSize, cpu, counts, times), maxfev=10000)

## Plot
def plot_fit():
    print 'Estimater parameters: ', v
    #print 'Real parameters: ', v_real
    probSize = linspace(xmin,xmax,n*5)
    cpus = array([1500, 2500, 3500])
    
    plots= []
    plots.append(plot(realProbSize,times,'ro')) 
    labels = ['real']
    
    for cpu in cpus:
        for count in range(1,5):
            plots.append(plot(probSize, runTime(v, probSize, cpu, count)))
            labels.append("%d:%d" % (cpu,count))
    
    legend([p[0] for p in plots], labels)


plot_fit()
show()
from pylab import *
from numpy import *
import numpy

from mpl_toolkits.mplot3d import Axes3D
from matplotlib import cm
from matplotlib.ticker import LinearLocator, FixedLocator, FormatStrFormatter
import matplotlib.pyplot as plt
import numpy as np

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



def run():
     
    ''' 
    points = [DataPoint(cpuCount=1, cpu=2, time=1000, probSize=35), 
              DataPoint(cpuCount=2, cpu=2, time=600, probSize=35),
              DataPoint(cpuCount=2, cpu=2, time=500, probSize=35), 
              DataPoint(cpuCount=2, cpu=2, time=575, probSize=35), 

              
              DataPoint(cpuCount=2, cpu=2, time=1100, probSize=70),
              DataPoint(cpuCount=2, cpu=2, time=1300, probSize=70)]
    '''
    def sec(hrFrac):
        return hrFrac * 60 * 60
        
    points = [
              #Local Data
              DataPoint(cpuCount=1, cpu=2, time=sec(0.07), probSize=85895824),
              DataPoint(cpuCount=2, cpu=2, time=sec(0.04), probSize=85895824),
              
              DataPoint(cpuCount=1, cpu=2, time=sec(0.08), probSize=100000000),
              DataPoint(cpuCount=2, cpu=2, time=sec(0.05), probSize=100000000),
              
              DataPoint(cpuCount=1, cpu=2, time=sec(0.27), probSize=199996164),
              DataPoint(cpuCount=2, cpu=2, time=sec(0.17), probSize=199996164),
              
              DataPoint(cpuCount=1, cpu=2, time=sec(0.76), probSize=374964496),
              DataPoint(cpuCount=2, cpu=2, time=sec(0.51), probSize=374964496),
              
              #EC2 Data
              DataPoint(cpuCount=2, cpu=3.25, time=sec(0.196), probSize=374964496),
              DataPoint(cpuCount=2, cpu=3.25, time=sec(1.86), probSize=1699995361),
              DataPoint(cpuCount=4, cpu=3.25, time=sec(2.28), probSize=2999971984),
              DataPoint(cpuCount=8, cpu=2, time=sec(1.77), probSize=2999971984),
              DataPoint(cpuCount=16, cpu=2, time=sec(0.93), probSize=2999971984),
              DataPoint(cpuCount=4, cpu=3.25, time=sec(2.72), probSize=3419910400),
              DataPoint(cpuCount=16, cpu=2, time=sec(1.84), probSize=4799995524),
              ]
    
    model = AmdahlsCompModel(dataPoints=points, scaleFunction='linear')
    '''
    plots= []
    plots.append(plot([dp.probSize for dp in points],[dp.time for dp in points],'ro')) 
    labels = ['real']

    probSize = linspace(0,200,5)

    for count in range(1,5):
        plots.append(plot(probSize, model.modelFunc(probSize, None, count)))
        labels.append("%d" % count)

        legend([p[0] for p in plots], labels)

    show()
    '''
    fig = plt.figure()
    ax = Axes3D(fig) #fig.gca(projection='3d')
    probSize = arange(1, 200, 5)
    numProcs = arange(1, 10, 1)
    
    probSize, numProcs = np.meshgrid(probSize, numProcs)
    
    time = model.modelFunc(probSize, 2, numProcs)
    
    colortuple = ('y', 'b')
    colors = np.empty(probSize.shape, dtype=str)
    for y in range(len(numProcs)):
        for x in range(len(probSize)):
            colors[x, y] = colortuple[(x + y) % len(colortuple)]
    
    
    surf = ax.plot_surface(probSize, numProcs, time, rstride=1, cstride=1, #facecolors=colors,
            linewidth=0, antialiased=False)
    
    '''surf = ax.plot_wireframe(probSize, numProcs, time, rstride=1, cstride=1,
             antialiased=False)
    '''
    for p in points:
        ax.scatter(array([p.probSize]), array([p.machineCount]), array([p.time]), c='r', marker='o')
            
    ax.set_zlim3d(0, sec(2.28))
    ax.w_zaxis.set_major_locator(LinearLocator(6))
    ax.w_zaxis.set_major_formatter(FormatStrFormatter('%.03f'))
    ax.w_yaxis.set_major_formatter(FormatStrFormatter('%d'))
    ax.w_xaxis.set_major_formatter(FormatStrFormatter('%.0f'))
    
    plt.show()
    
if __name__ == "__main__":
    run()

    
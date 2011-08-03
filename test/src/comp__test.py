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



def run():
     
     
    points = [DataPoint(cpuCount=1, cpu=1, time=1000, probSize=35), 
              DataPoint(cpuCount=2, cpu=1, time=600, probSize=35),
              DataPoint(cpuCount=2, cpu=1, time=500, probSize=35), 
              DataPoint(cpuCount=2, cpu=1, time=575, probSize=35), 

              
              DataPoint(cpuCount=2, cpu=1, time=1100, probSize=70),
              DataPoint(cpuCount=2, cpu=1, time=1300, probSize=70)]
    
    model = AmdahlsCompModel(dataPoints=points, scaleFunction='linear')
    
    plots= []
    plots.append(plot([dp.probSize for dp in points],[dp.time for dp in points],'ro')) 
    labels = ['real']

    probSize = linspace(0,200,5)

    for count in range(1,5):
        plots.append(plot(probSize, model.modelFunc(probSize, None, count)))
        labels.append("%d" % count)

        legend([p[0] for p in plots], labels)

    show()

if __name__ == "__main__":
    run()

    
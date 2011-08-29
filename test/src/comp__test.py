from pylab import *
from numpy import *
import numpy
import traceback

from mpl_toolkits.mplot3d import Axes3D

from d2c.model.CompModel import AmdahlsCompModel, SimpleDataPoint, GustafsonCompModel, PolyCompModel3, GustafsonCompModel2

def run():
     
    def sec(hrFrac):
        return hrFrac * 60 * 60
   
    
    vboxCpu = 2
    vboxPoints = [
                  
              #Local Data
              SimpleDataPoint(cpuCount=1, cpu=vboxCpu, time=sec(0.07), probSize=85.895824, totalMemory=12),
              SimpleDataPoint(cpuCount=2, cpu=vboxCpu, time=sec(0.04), probSize=85.895824, totalMemory=12),
              
              SimpleDataPoint(cpuCount=1, cpu=vboxCpu, time=sec(0.08), probSize=100.000000, totalMemory=12),
              SimpleDataPoint(cpuCount=2, cpu=vboxCpu, time=sec(0.05), probSize=100.000000, totalMemory=12),
              
              SimpleDataPoint(cpuCount=1, cpu=vboxCpu, time=sec(0.27), probSize=199.996164, totalMemory=12),
              SimpleDataPoint(cpuCount=2, cpu=vboxCpu, time=sec(0.17), probSize=199.996164, totalMemory=12),
              
              SimpleDataPoint(cpuCount=1, cpu=vboxCpu, time=sec(0.76), probSize=374.964496, totalMemory=12),
              SimpleDataPoint(cpuCount=2, cpu=vboxCpu, time=sec(0.51), probSize=374.964496, totalMemory=12)
                  
                  ]
    
   
    ec2Points = [
              
              #EC2 Data
              SimpleDataPoint(cpuCount=2, cpu=3.25, time=sec(0.196), probSize=374.964496, totalMemory=12),
              SimpleDataPoint(cpuCount=2, cpu=3.25, time=sec(1.86), probSize=1699.995361, totalMemory=12),
              SimpleDataPoint(cpuCount=4, cpu=3.25, time=sec(2.28), probSize=2999.971984, totalMemory=12),
              SimpleDataPoint(cpuCount=8, cpu=2, time=sec(1.77), probSize=2999.971984, totalMemory=12),
              SimpleDataPoint(cpuCount=16, cpu=2, time=sec(0.93), probSize=2999.971984, totalMemory=12),
              SimpleDataPoint(cpuCount=4, cpu=3.25, time=sec(2.72), probSize=3419.910400, totalMemory=12),
              SimpleDataPoint(cpuCount=16, cpu=2, time=sec(1.84), probSize=4799.995524, totalMemory=12),
              ]
    
    #modelPoints = points[:15]
    
    
    c = 2.
    for p in vboxPoints:
        p.cpu = c
    
    trainingPoints = []
    trainingPoints.extend(ec2Points)
    trainingPoints.extend(vboxPoints)
    
    try:
        model = GustafsonCompModel2(dataPoints=trainingPoints, scaleFunction='linear')
        #model = PolyCompModel3(dataPoints=trainingPoints, scaleFunction='linear')
    except:
        print "Exception ", sys.exc_info()[0]
        traceback.print_exc()
        
    rating = model.modelSumOfSquares(trainingPoints)
    print "rating = %f" %  rating  
        
    modelPoints = []
    modelPoints.extend(ec2Points)
    modelPoints.extend(vboxPoints)
    

    fig = plt.figure()
    ax = Axes3D(fig) #fig.gca(projection='3d')
    maxProbSize = max([dp.probSize for dp in modelPoints]) * 1.2
    
    probSize = arange(min([dp.probSize for dp in modelPoints]), maxProbSize, 5)
    numProcs = arange(1, 19, 1)
    
    probSize, numProcs = np.meshgrid(probSize, numProcs)
    
    time = model.modelFunc(probSize, 1, numProcs)
    
 
    rstride = 1#(max([dp.machineCount for dp in points]) - min([dp.machineCount for dp in points])) / 20
    cstride = int(math.ceil(true_divide((max([dp.probSize for dp in modelPoints]) - min([dp.probSize for dp in modelPoints])), 45)))
    
    surf = ax.plot_wireframe(probSize, numProcs, time, rstride=rstride, cstride=cstride,
             antialiased=False)
    
    for p in modelPoints[:7]:
        ''' Note the normalization of time by cpu speed '''
        ax.scatter(array([p.probSize]), array([p.machineCount]), array([p.time * p.cpu]), c='r', marker='o')
    
    for p in modelPoints[7:]:
        ''' Note the normalization of time by cpu speed '''
        ax.scatter(array([p.probSize]), array([p.machineCount]), array([p.time * p.cpu]), c='y', marker='o')
            
    ax.set_zlim3d(0, max([p.time * p.cpu for p in modelPoints]))
    ax.set_xlim3d(min([p.probSize for p in modelPoints]), maxProbSize)
    ax.w_zaxis.set_major_locator(LinearLocator(6))
    ax.w_zaxis.set_major_formatter(FormatStrFormatter('%.03f'))
    ax.w_yaxis.set_major_formatter(FormatStrFormatter('%d'))
    ax.w_xaxis.set_major_formatter(FormatStrFormatter('%d'))
    
    ax.set_xlabel('Problem Size')
    ax.set_ylabel('CPU Count')
    ax.set_zlabel('Time')

    
    #savefig("test.png")
    plt.show()
    
if __name__ == "__main__":
    run()

    
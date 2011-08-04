from pylab import *
from numpy import *
import numpy

from mpl_toolkits.mplot3d import Axes3D



from d2c.model.CompModel import AmdahlsCompModel, DataPoint



def run():
     
    '''
    points = [DataPoint(cpuCount=1, cpu=2, time=1000, probSize=35), 
              DataPoint(cpuCount=2, cpu=2, time=600, probSize=35),
              DataPoint(cpuCount=2, cpu=2, time=500, probSize=35), 
              DataPoint(cpuCount=2, cpu=2, time=575, probSize=35), 

              
              DataPoint(cpuCount=2, cpu=2, time=1100, probSize=70),
              DataPoint(cpuCount=2, cpu=2, time=1300, probSize=70)]
    '''
    '''
    points = [DataPoint(cpuCount=1, cpu=2, time=1000, probSize=350), 
              DataPoint(cpuCount=2, cpu=2, time=600, probSize=350),
              DataPoint(cpuCount=2, cpu=2, time=500, probSize=350), 
              DataPoint(cpuCount=2, cpu=2, time=575, probSize=350), 

              
              DataPoint(cpuCount=2, cpu=2, time=1100, probSize=700),
              DataPoint(cpuCount=2, cpu=2, time=1300, probSize=700)]
    '''
    
    def sec(hrFrac):
        return hrFrac * 60 * 60
    '''
    points = [
              #Local Data
              DataPoint(cpuCount=1, cpu=2, time=sec(0.07), probSize=85895824),
              DataPoint(cpuCount=2, cpu=2, time=sec(0.04), probSize=85895824),
              
              DataPoint(cpuCount=1, cpu=2, time=sec(0.08), probSize=100000000),
              #DataPoint(cpuCount=2, cpu=2, time=sec(0.05), probSize=100000000),
              
              #DataPoint(cpuCount=1, cpu=2, time=sec(0.27), probSize=199996164),
              #DataPoint(cpuCount=2, cpu=2, time=sec(0.17), probSize=199996164),
              
              #DataPoint(cpuCount=1, cpu=2, time=sec(0.76), probSize=374964496),
              #DataPoint(cpuCount=2, cpu=2, time=sec(0.51), probSize=374964496),
              
              #EC2 Data
              #DataPoint(cpuCount=2, cpu=3.25, time=sec(0.196), probSize=374964496),
              #DataPoint(cpuCount=2, cpu=3.25, time=sec(1.86), probSize=1699995361),
              #DataPoint(cpuCount=4, cpu=3.25, time=sec(2.28), probSize=2999971984),
              #DataPoint(cpuCount=8, cpu=2, time=sec(1.77), probSize=2999971984),
              #DataPoint(cpuCount=16, cpu=2, time=sec(0.93), probSize=2999971984),
              #DataPoint(cpuCount=4, cpu=3.25, time=sec(2.72), probSize=3419910400),
              #DataPoint(cpuCount=16, cpu=2, time=sec(1.84), probSize=4799995524),
              ]
    '''
    
    points = [
              #Local Data
              DataPoint(cpuCount=1, cpu=2, time=sec(0.07), probSize=85.895824),
              DataPoint(cpuCount=2, cpu=2, time=sec(0.04), probSize=85.895824),
              
              DataPoint(cpuCount=1, cpu=2, time=sec(0.08), probSize=100.000000),
              DataPoint(cpuCount=2, cpu=2, time=sec(0.05), probSize=100.000000),
              
              DataPoint(cpuCount=1, cpu=2, time=sec(0.27), probSize=199.996164),
              DataPoint(cpuCount=2, cpu=2, time=sec(0.17), probSize=199.996164),
              
              DataPoint(cpuCount=1, cpu=2, time=sec(0.76), probSize=374.964496),
              DataPoint(cpuCount=2, cpu=2, time=sec(0.51), probSize=374.964496),
              
              #EC2 Data
              DataPoint(cpuCount=2, cpu=3.25, time=sec(0.196), probSize=374.964496),
              DataPoint(cpuCount=2, cpu=3.25, time=sec(1.86), probSize=1699.995361),
              DataPoint(cpuCount=4, cpu=3.25, time=sec(2.28), probSize=2999.971984),
              DataPoint(cpuCount=8, cpu=2, time=sec(1.77), probSize=2999.971984),
              DataPoint(cpuCount=16, cpu=2, time=sec(0.93), probSize=2999.971984),
              DataPoint(cpuCount=4, cpu=3.25, time=sec(2.72), probSize=3419.910400),
              DataPoint(cpuCount=16, cpu=2, time=sec(1.84), probSize=4799.995524),
              ]
    
    modelPoints = points[:15]
    
    
    minRating = sys.maxint
    model = None
    bestBasis = None
    
    for c in range(0, len(points)):
        modelPoints = points[:c]
        try:
            m = AmdahlsCompModel(dataPoints=modelPoints, scaleFunction='linear')
        except:
            continue
        rating = m.modelSumOfSquares(points)
        print "basis = %d; diff = %f" %  (c, rating)
        if rating < minRating:
            model = m
            minRating = rating
            bestBasis = c
    
    print "\nBest basis = %d; diff = %f" %  (bestBasis, minRating)
    
    
    
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
    maxProbSize = max([dp.probSize for dp in points]) * 1.2
    
    probSize = arange(min([dp.probSize for dp in points]), maxProbSize, 5)
    numProcs = arange(1, 16, 1)
    
    probSize, numProcs = np.meshgrid(probSize, numProcs)
    
    time = model.modelFunc(probSize, 2, numProcs)
    
    colortuple = ('y', 'b')
    colors = np.empty(probSize.shape, dtype=str)
    #for y in range(len(numProcs)):
    #    for x in range(len(probSize)):
    #        colors[x, y] = colortuple[(x + y) % len(colortuple)]
    
    
    #surf = ax.plot_surface(probSize, numProcs, time, rstride=1, cstride=1, #facecolors=colors,
    #        linewidth=0, antialiased=False)
    
    rstride = 1#(max([dp.machineCount for dp in points]) - min([dp.machineCount for dp in points])) / 20
    cstride = int(math.ceil(true_divide((max([dp.probSize for dp in points]) - min([dp.probSize for dp in points])), 45)))
    
    surf = ax.plot_wireframe(probSize, numProcs, time, rstride=rstride, cstride=cstride,
             antialiased=False)
    
    for p in points:
        ax.scatter(array([p.probSize]), array([p.machineCount]), array([p.time]), c='r', marker='o')
            
    ax.set_zlim3d(0, max([p.time for p in points]))
    ax.set_xlim3d(min([p.probSize for p in points]), maxProbSize)
    ax.w_zaxis.set_major_locator(LinearLocator(6))
    ax.w_zaxis.set_major_formatter(FormatStrFormatter('%.03f'))
    ax.w_yaxis.set_major_formatter(FormatStrFormatter('%d'))
    ax.w_xaxis.set_major_formatter(FormatStrFormatter('%d'))
    
    plt.show()
    
if __name__ == "__main__":
    run()

    
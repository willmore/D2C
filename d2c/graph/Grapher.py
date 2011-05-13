'''
Created on May 13, 2011

@author: willmore
'''
import rrdtool
import re
import os
import string

class Grapher:
    
    def __init__(self, srcDir, destDir):
        self.srcDir = srcDir
        self.destDir = destDir
        
    def generateGraphs(self, start, end):
        pass
        
    def generateCPUGraphs(self, start, end):
        
        imageName = os.path.join(self.destDir, "CPU.png")
        
        dataSets = {}
        cpuDirRegex = re.compile("cpu\-(\d+)")
        
        for hostName in [d for d in os.listdir(self.srcDir)
                            if os.path.isdir(os.path.join(self.srcDir, d))]:
           
            print hostName 
            hostDir = os.path.join(self.srcDir, hostName)
        
            for dir in [d for d in os.listdir(hostDir) 
                            if os.path.isdir(os.path.join(hostDir, d))]:
                                
                m = cpuDirRegex.match(dir)
                if m is None:
                    continue
                cpuId = m.group(1)
                rrdFile = os.path.join(hostDir, dir, "cpu-user.rrd")
                ds = '%s-cpu%s' % (hostName, cpuId)
                dataSets[ds] = 'DEF:%s=%s:value:AVERAGE' % (ds, rrdFile)
        
        print dataSets
        
        lines = []
        lineCnt = 1
        for ds in dataSets.keys():
            lines.append("LINE%d:%s#0000FF:%s" % (lineCnt, ds, ds))
            lineCnt += 1
        
        args = [imageName,
                '--imgformat', 'PNG',
             '--width', '540',
             '--height', '100',
             '--start', start,
             '--end', end,
             '--vertical-label', 'CPU',
             '--title', 'CPU',
             '--lower-limit', '0',]
        args.extend(dataSets.values())
        args.extend(lines)
        
        print args
        apply(rrdtool.graph, args)
        
        return imageName
    
    def generateCPUGraphs2(self, start, end):
        
        imageName = os.path.join(self.destDir, "CPU.png")
        
        userDataSets = {}
        systemDataSets = {}
        cpuDirRegex = re.compile("cpu\-(\d+)")
        
        for hostName in [d for d in os.listdir(self.srcDir)
                            if os.path.isdir(os.path.join(self.srcDir, d))]:
           
            print hostName 
            hostDir = os.path.join(self.srcDir, hostName)
        
            for dir in [d for d in os.listdir(hostDir) 
                            if os.path.isdir(os.path.join(hostDir, d))]:
                                
                m = cpuDirRegex.match(dir)
                if m is None:
                    continue
                cpuId = m.group(1)
                rrdFile = os.path.join(hostDir, dir, "cpu-user.rrd")
                ds = '%s-user-cpu%s' % (hostName, cpuId)
                userDataSets[ds] = 'DEF:%s=%s:value:AVERAGE' % (ds, rrdFile)
                
                rrdFile = os.path.join(hostDir, dir, "cpu-system.rrd")
                ds = '%s-system-cpu%s' % (hostName, cpuId)
                systemDataSets[ds] = 'DEF:%s=%s:value:AVERAGE' % (ds, rrdFile)
        
        #print dataSets
        
        cdef = "CDEF:cpu-user=%s,%d,AVG" % (string.join(userDataSets.keys(), ","), 
                                            len(userDataSets))
        cdefSystem = "CDEF:cpu-system=%s,%d,AVG" % (string.join(systemDataSets.keys(), ","), 
                                            len(systemDataSets))
        #defTotal = "CDEF:cpu-total=cpu-user,cpu-system,+"
        
        #lines = ["LINE1:cpu-user#0000FF:cpu-user",
        #        "LINE2:cpu-system#FF0000:cpu-system"]
        
        lines = ["AREA:cpu-user#0000FF::",
                "AREA:cpu-system#FF0000::STACK"]
    
        #lines = ["LINE1:cpu-user#0000FF:cpu-user"]
        
        args = [imageName,
                '--imgformat', 'PNG',
             '--width', '540',
             '--height', '100',
             '--start', start,
             '--end', end,
             '--vertical-label', 'CPU',
             '--title', 'CPU',
             '--lower-limit', '0',]
        args.extend(userDataSets.values())
        args.extend(systemDataSets.values())
        args.append(cdef)
        args.append(cdefSystem)
        args.extend(lines)
        
        print args
        apply(rrdtool.graph, args)
        
        return imageName
        
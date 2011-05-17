'''
Created on May 13, 2011

@author: willmore
'''
import rrdtool
import re
import os
import string

class Grapher:
    
    def __init__(self, srcs, destDir, imageWidth=540, imageHeight=100):
        self.srcs = srcs # Map of hostId => directory
        self.destDir = destDir
        self.imageWidth = str(imageWidth)
        self.imageHeight = str(imageHeight)
        
    def generateGraphs(self, start, end):
        pass

    colors = ['000000','7B0000','990000','BB0000','CC0000','D90000','EE0000','FF0000','CC0000',
             '000000','7B7B7B','999999','BBBBBB','CCCCCC','D9D9D9','EEEEEE','FFFFFF','CCCCCC']
    
    def generateMemoryGraph(self, start, end):
        '''
        Generate one area:
            a stack for total memory free and one stack for total memory used.
        '''
        imageName = os.path.join(self.destDir, "Memory.png")
        
        usedDataSets = {}
        freeDataSets = {}
        memDirRegex = re.compile(".*memory$")
        
        for (hostName,hostDir) in self.srcs.iteritems():
           
            for statDir in [os.path.join(hostDir, d) for d in os.listdir(hostDir)]:                
                m = memDirRegex.match(statDir)
                
                if m is None:
                    continue
                
                rrdFile = os.path.join(statDir, "memory-used.rrd")
                ds = '%s-mem-used' % hostName
                usedDataSets[ds] = 'DEF:%s=%s:value:AVERAGE' % (ds, rrdFile)
                
                rrdFile = os.path.join(statDir, "memory-free.rrd")
                ds = '%s-mem-free' % hostName
                freeDataSets[ds] = 'DEF:%s=%s:value:AVERAGE' % (ds, rrdFile)
        
        cdef = "CDEF:mem-used=%s,%s" % (string.join(usedDataSets.keys(), ","),
                                        string.join(['+' for _ in range(len(usedDataSets)-1)], ","))
        cdefSystem = "CDEF:mem-free=%s,%s" % (string.join(freeDataSets.keys(), ","),
                                              string.join(['+' for _ in range(len(freeDataSets)-1)], ","))
        
        lines = ["AREA:mem-used#0000FF:Memory Used:",
                "AREA:mem-free#FF0000:Memory Free:STACK"]
        
        args = [imageName,
                '--imgformat', 'PNG',
                 '--width', self.imageWidth,
                 '--height', self.imageHeight,
                 '--start', start,
                 '--end', end,
                 '--vertical-label', 'Memory',
                 '--title', 'Memory Across Cluser',
                 '--lower-limit', '0',]
        
        args.extend(usedDataSets.values())
        args.extend(freeDataSets.values())
        args.append(cdef)
        args.append(cdefSystem)
        args.extend(lines)

        apply(rrdtool.graph, args)
        
        return imageName
    
    def generateNetworkGraph(self, start, end):
        '''
        Generate two lines per host: one for TX and one for RX
        '''
        
        imageName = os.path.join(self.destDir, "TotalNetwork.png")
        
        pxRegex = re.compile(".*if_packets-(.*).rrd$")
        
        dataSets = {}
        cdefSets = {}
        
        for (hostName,hostDir) in self.srcs.iteritems():
           
            interfaceDir = os.path.join(hostDir, "interface")
            
            if not os.path.isdir(os.path.join(interfaceDir)):
                continue

            txSets = {}
            rxSets = {}

            for pxRRD in [os.path.join(interfaceDir, d) 
                            for d in os.listdir(interfaceDir) 
                                if pxRegex.match(d)]:
                                                    
                device = pxRegex.match(pxRRD).group(1)
                
                ds = '%s-%s-tx' % (hostName, device)
                txSets[ds] = 'DEF:%s=%s:tx:AVERAGE' % (ds, pxRRD)
                
                ds = '%s-%s-rx' % (hostName, device)
                rxSets[ds] = 'DEF:%s=%s:rx:AVERAGE' % (ds, pxRRD)
                
            for (k,v) in txSets.iteritems():
                dataSets[k] = v
                    
            for (k,v) in rxSets.iteritems():
                dataSets[k] = v
            
            
            cdefName = "%s-TX" % hostName
            cdefSets[cdefName] = "CDEF:%s=%s,%d,AVG" % (cdefName, 
                                                        string.join(txSets.keys(), ","), 
                                                        len(txSets))
            
            cdefName = "%s-RX" % hostName
            cdefSets[cdefName] = "CDEF:%s=%s,%d,AVG" % (cdefName, 
                                                        string.join(rxSets.keys(), ","), 
                                                        len(rxSets))
            
        lines = []
        
        for i, name in enumerate(cdefSets.keys()):
            lines.append("LINE%d:%s#%s:%s" % (i, name, self.colors[i], name))
        
        args = [imageName,
                '--imgformat', 'PNG',
                 '--width', self.imageWidth,
                 '--height', self.imageHeight,
                 '--start', start,
                 '--end', end,
                 '--vertical-label', 'Packet Count',
                 '--title', 'Network Traffic in Cluster',
                 '--lower-limit', '0',]
        
        args.extend(dataSets.values())
        args.extend(cdefSets.values())
        args.extend(lines)
        
        apply(rrdtool.graph, args)
        
        return imageName
      
      
    def generateLoadGraph(self, start, end):
        '''
        Graph 3 line per host:
            shortterm, midterm, and longterm
        '''
        
        imageName = os.path.join(self.destDir, "Load.png")
        
        dataSets = {}
        
        for (hostName,hostDir) in self.srcs.iteritems():

            rrdFile = os.path.join(hostDir, "load", "load.rrd")
            
            if not os.path.isfile(rrdFile):
                continue
            
            ds = '%s-shortterm' % hostName
            dataSets[ds] = 'DEF:%s=%s:shortterm:AVERAGE' % (ds, rrdFile)
            
            ds = '%s-midterm' % hostName
            dataSets[ds] = 'DEF:%s=%s:midterm:AVERAGE' % (ds, rrdFile)
            
            ds = '%s-longterm' % hostName
            dataSets[ds] = 'DEF:%s=%s:longterm:AVERAGE' % (ds, rrdFile)
        
        
        lines = []
        lineCnt = 1
        for i, ds in enumerate(dataSets.keys()):
            lines.append("LINE%d:%s#%s:%s" % (lineCnt, ds, self.colors[i], ds))
            lineCnt += 1
        
        args = [imageName,
                '--imgformat', 'PNG',
             '--width', self.imageWidth,
             '--height', self.imageHeight,
             '--start', start,
             '--end', end,
             '--vertical-label', 'Load',
             '--title', 'Host Load',
             '--lower-limit', '0']
        
        args.extend(dataSets.values())
        args.extend(lines)
        
        apply(rrdtool.graph, args)
        
        return imageName  
        
    def generateCPUGraphs(self, start, end):
        
        imageName = os.path.join(self.destDir, "CPU.png")
        
        dataSets = {}
        cpuDirRegex = re.compile("cpu\-(\d+)")
        
        for hostName in [d for d in os.listdir(self.srcDir)
                            if os.path.isdir(os.path.join(self.srcDir, d))]:
           
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
        
        
        lines = []
        lineCnt = 1
        for ds in dataSets.keys():
            lines.append("LINE%d:%s#0000FF:%s" % (lineCnt, ds, ds))
            lineCnt += 1
        
        args = [imageName,
                '--imgformat', 'PNG',
             '--width', self.imageWidth,
             '--height', self.imageHeight,
             '--start', start,
             '--end', end,
             '--vertical-label', 'CPU',
             '--title', 'CPU',
             '--lower-limit', '0',]
        args.extend(dataSets.values())
        args.extend(lines)
        
        apply(rrdtool.graph, args)
        
        return imageName
    
    def generateCPUGraphsAverage(self, start, end):
        '''
        Generate one area for average host user CPU and one stack for average System CPU
        '''
        
        imageName = os.path.join(self.destDir, "CPU.png")
        
        userDataSets = {}
        systemDataSets = {}
        stealDataSets = {}
        cpuDirRegex = re.compile(".*cpu\-(\d+)$")
        
        for (hostName,hostDir) in self.srcs.iteritems():
           
            for statDir in [os.path.join(hostDir, d) for d in os.listdir(hostDir)]:                
                m = cpuDirRegex.match(statDir)

                if m is None:
                    continue

                cpuId = m.group(1)
                rrdFile = os.path.join(statDir, "cpu-user.rrd")
                ds = '%s-user-cpu%s' % (hostName, cpuId)
                userDataSets[ds] = 'DEF:%s=%s:value:AVERAGE' % (ds, rrdFile)
                
                rrdFile = os.path.join(statDir, "cpu-system.rrd")
                ds = '%s-system-cpu%s' % (hostName, cpuId)
                systemDataSets[ds] = 'DEF:%s=%s:value:AVERAGE' % (ds, rrdFile)
                
                rrdFile = os.path.join(statDir, "cpu-steal.rrd")
                ds = '%s-steal-cpu%s' % (hostName, cpuId)
                stealDataSets[ds] = 'DEF:%s=%s:value:AVERAGE' % (ds, rrdFile)
        
        
        cdef = "CDEF:cpu-user=%s,%d,AVG" % (string.join(userDataSets.keys(), ","), 
                                            len(userDataSets))
        cdefSystem = "CDEF:cpu-system=%s,%d,AVG" % (string.join(systemDataSets.keys(), ","), 
                                            len(systemDataSets))
        
        cdefSteal = "CDEF:cpu-steal=%s,%d,AVG" % (string.join(stealDataSets.keys(), ","), 
                                             len(stealDataSets))
        
        lines = ["AREA:cpu-user#00FF00:CPU User:",
                "AREA:cpu-system#0000FF:CPU System:STACK",
                "AREA:cpu-steal#FF0000:CPU Steal:STACK"]
            
        args = [imageName,
                '--imgformat', 'PNG',
                 '--width', self.imageWidth,
                 '--height', self.imageHeight,
                 '--start', start,
                 '--end', end,
                 '--vertical-label', 'CPU Percentage',
                 '--title', 'CPU Usage Across Cluster',
                 '--lower-limit', '0',]
        
        args.extend(userDataSets.values())
        args.extend(systemDataSets.values())
        args.extend(stealDataSets.values())
        args.append(cdef)
        args.append(cdefSystem)
        args.append(cdefSteal)
        args.extend(lines)
        
        apply(rrdtool.graph, args)
        
        return imageName
        
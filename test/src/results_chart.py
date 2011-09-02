
import re
from pylab import *
file = open("/home/willmore/test10k.txt", "r")

def color(clientCnt):
    if clientCnt <=7:
        return 'b'
    if clientCnt <=14:
        return 'r'
    return 'g'

line1 = True
for line in file:
    if line1:
        m = re.match("workerCnt (\d+) clientCnt (\d+)", line)
        workerCnt, clientCnt = (int(m.group(1)),int(m.group(2)))
        print workerCnt, " ", clientCnt
        line1 = False;
    else:
        m = re.search("(\d+\.\d+),", line)
        throughput = float(m.group(1))
        
        scatter(workerCnt, throughput,c=color(clientCnt))
        line1 = True

xlabel("Worker Count")
ylabel("Throughput (req / s)")
show()
    
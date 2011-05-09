'''
Created on Feb 23, 2011

@author: willmore
'''
import time

class DevNullLogger():
    """
    Logger that writes to nowhere
    """
    
    def write(self, msg):
        return None
    
class StdOutLogger():
    
    def write(self, msg):
        print "[%d] %s" % (time.time(), msg)
        
'''
Created on Feb 23, 2011

@author: willmore
'''
class DevNullLogger():
    """
    Logger that writes to nowhere
    """
    
    def write(self, msg):
        return None
    
class StdOutLogger():
    
    def write(self, msg):
        print msg
        
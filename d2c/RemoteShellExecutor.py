'''
Created on Apr 12, 2011

@author: willmore
'''

from .ShellExecutor import ShellExecutor
from .logger import StdOutLogger

import string

class RemoteShellExecutorFactory(object):

    def executor(self, user, host, 
                 privateKey=None, 
                 outputLogger=StdOutLogger(),
                 logger=StdOutLogger()):
        
        return RemoteShellExecutor(user, host, 
                                   privateKey, 
                                   outputLogger,
                                   logger)

class RemoteShellExecutor(ShellExecutor):
    
    def __init__(self, user, host, 
                 privateKey=None, 
                 outputLogger=StdOutLogger(),
                 logger=StdOutLogger()):
        
        ShellExecutor.__init__(self, outputLogger, logger)
        self.user = user
        self.host = host
        self.privateKey = privateKey
        
    def run(self, cmd):
        
        pKeyStr = "-i %s" % self.privateKey if self.privateKey else ""
        cmd = string.replace(cmd, "\\", "\\\\")
        cmd = string.replace(cmd, "\"", "\\\"")
        cmd = "ssh %s -o StrictHostKeyChecking=no %s@%s \"%s\"" % (pKeyStr, 
                                                                 self.user,
                                                                 self.host,
                                                                 cmd)
        
        ShellExecutor.run(self, cmd)
    
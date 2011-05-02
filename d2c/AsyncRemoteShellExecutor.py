'''
Created on Apr 12, 2011

@author: willmore
'''

from .RemoteShellExecutor import RemoteShellExecutor
from .ShellExecutor import ShellExecutor
from .logger import StdOutLogger

class AsyncRemoteShellExecutor(RemoteShellExecutor):
    
    def __init__(self, user, host, 
                 privateKey=None, 
                 outputLogger=StdOutLogger(),
                 logger=StdOutLogger()):
        
        RemoteShellExecutor.__init__(self, user, host, 
                                    privateKey, 
                                    outputLogger,
                                    logger)
        
    def run(self, cmd):
        '''
        Executes remote commands in a nohup sh wrapper. The run method will not block, and cmd 
        will continue on the remote machine.
        '''
        
        
        pKeyStr = "-i %s" % self.privateKey if self.privateKey else ""
        
        CMD_WRAPPER = "nohup sh -c \\\"%s\\\" &>/dev/null < /dev/null &"
        
        cmd = "ssh %s -o StrictHostKeyChecking=no %s@%s \"%s\"" % (pKeyStr, 
                                                                 self.user,
                                                                 self.host,
                                                                 CMD_WRAPPER % cmd)
        
        ShellExecutor.run(self, cmd)
    
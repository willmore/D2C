'''
Created on Apr 12, 2011

@author: willmore
'''

from .RemoteShellExecutor import RemoteShellExecutor
from .ShellExecutor import ShellExecutor
from .logger import StdOutLogger

import string
import random

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
                
        cmd = string.replace(cmd, "\\", "\\\\")
        cmd = string.replace(cmd, "\"", "\\\"")
        
        CMD_WRAPPER = "nohup sh -c \"%s\" &>/tmp/hup.%d.out < /dev/null &"
        
        cmd = CMD_WRAPPER % (cmd, random.randint(1,10000))
        
        RemoteShellExecutor.run(self, cmd)
       
from d2c.logger import StdOutLogger
from d2c.AsyncRemoteShellExecutor import AsyncRemoteShellExecutor
from .Action import Action

class AsyncAction(Action):

    def __init__(self, 
                 command, 
                 sshCred,
                 logger=StdOutLogger()):
        
        Action.__init__(self, command, sshCred, logger)
     
    def execute(self, instance):   
            
        AsyncRemoteShellExecutor(self.sshCred.username, 
                            instance.public_dns_name, 
                            self.sshCred.privateKey).run(self.command)

    def copy(self):
        return AsyncAction(self.command, self.sshCred)

from d2c.logger import StdOutLogger
from d2c.RemoteShellExecutor import RemoteShellExecutor

class Action(object):

    def __init__(self, 
                 command, 
                 sshCred,
                 logger=StdOutLogger()):
        
        assert isinstance(command, basestring)
        
        self.command = command
        self.sshCred = sshCred
        self.logger = logger
        
    def copy(self):
        return Action(self.command, self.sshCred)
     
    def execute(self, instance):   
            
        self.remoteExecutorFactory.executor(self.sshCred.username, 
                            instance.public_dns_name, 
                            self.sshCred.privateKey).run(self.command)
                            
                            
class StartAction(Action):

    def __init__(self, 
                 command, 
                 sshCred,
                 logger=StdOutLogger()):
        
        Action.__init__(self, command, 
                 sshCred,
                 logger)
        
    def copy(self):
        return StartAction(self.command, self.sshCred)
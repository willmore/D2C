from d2c.logger import StdOutLogger
from d2c.RemoteShellExecutor import RemoteShellExecutor

class Action():

    def __init__(self, 
                 command, 
                 sshCred,
                 logger=StdOutLogger()):
        
        assert isinstance(command, basestring)
        
        self.command = command
        self.sshCred = sshCred
        self.logger = logger
     
    def execute(self, instance):   
            
        RemoteShellExecutor(self.sshCred.username, 
                            instance.public_dns_name, 
                            self.sshCred.privateKey).run(self.command)

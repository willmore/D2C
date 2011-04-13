from d2c.ShellExecutor import ShellExecutor
from d2c.logger import StdOutLogger

class Action():

    def __init__(self, command, 
                 credStore=None,
                 logger=StdOutLogger()):
        
        assert isinstance(command, str)
        
        self.command = command
        self.credStore = credStore
        self.logger = logger
     
    def execute(self, instance):   
        cred = self.credStore.getEC2Cred(instance.key_name)
            
        cmd = "rsh -i %s -o StrictHostKeyChecking=no ec2-user@%s '%s'" % (cred.private_key, 
                                                                          instance.public_dns_name, 
                                                                          self.command)
        ShellExecutor(self.logger).run(cmd)
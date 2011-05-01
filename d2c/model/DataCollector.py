from d2c.ShellExecutor import ShellExecutor
from d2c.logger import StdOutLogger
import os


class DataCollector():

    def __init__(self, 
                 source, 
                 destination, 
                 sshCred,
                 logger=StdOutLogger()):
        
        assert source is not None
        assert destination is not None
        
        self.source = source
        self.destination = destination
        self.logger = logger
        self.sshCred = sshCred
     
    def collect(self, instance): 
        
        if not os.path.exists(os.path.dirname(self.destination)):
            os.makedirs(os.path.dirname(self.destination))
             
        cmd = "scp -r -i %s -o StrictHostKeyChecking=no %s@%s:%s %s" % (self.sshCred.privateKey, 
                                                                        self.sshCred.username,
                                                                        instance.public_dns_name, 
                                                                        self.source,
                                                                        self.destination)
        ShellExecutor(self.logger).run(cmd)
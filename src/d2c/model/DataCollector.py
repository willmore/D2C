from d2c.ShellExecutor import ShellExecutor
from d2c.logger import StdOutLogger
import os


class DataCollector():

    def __init__(self, source, destination, 
                 credStore=None, 
                 user='ec2-user',
                 logger=StdOutLogger()):
        
        assert source is not None
        assert destination is not None
        
        self.source = source
        self.destination = destination
        self.logger = logger
        self.credStore = credStore
        self.user = user
     
    def collect(self, instance): 
        
        if not os.path.exists(os.path.dirname(self.destination)):
            os.makedirs(os.path.dirname(self.destination))
          
        cred = self.credStore.getEC2Cred(instance.key_name)
            
        cmd = "scp -r -i %s -o StrictHostKeyChecking=no %s@%s:%s %s" % (cred.private_key, 
                                                                        self.user,
                                                                        instance.public_dns_name, 
                                                                        self.source,
                                                                        self.destination)
        ShellExecutor(self.logger).run(cmd)
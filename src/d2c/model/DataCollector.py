from d2c.ShellExecutor import ShellExecutor
from d2c.logger import StdOutLogger

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
        cred = self.credStore.getEC2Cred(instance.key_name)
            
        cmd = "scp -r -i %s -o StrictHostKeyChecking=no %s@%s:%s %s" % (cred.private_key, 
                                                                        self.user,
                                                                        instance.public_dns_name, 
                                                                        self.source,
                                                                        self.destination)
        ShellExecutor(self.logger).run(cmd)
from d2c.ShellExecutor import ShellExecutor
from d2c.logger import StdOutLogger

class DataCollector():

    def __init__(self, source, destination, dao=None, logger=StdOutLogger()):
        assert source is not None
        assert destination is not None
        
        self.source = source
        self.destination = destination
        self.logger = logger
        self.dao = dao
     
    def execute(self, instance):   
        cred = self.dao.getEC2Cred(instance.key_name)
            
        cmd = "scp -r -i %s -o StrictHostKeyChecking=no ec2-user@%s:%s %s" % (cred.private_key, 
                                                                          instance.public_dns_name, 
                                                                          self.source,
                                                                          self.destination)
        ShellExecutor(self.logger).run(cmd)
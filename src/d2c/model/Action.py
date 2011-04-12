from d2c.ShellExecutor import ShellExecutor

class Action():

    def __init__(self, command):
        assert command is not None
        
        self.command = command
        
     
    def execute(self, instance):   
        cred = self.dao.getEC2Cred(instance.key_name)
            
        cmd = "rsh -i %s -o StrictHostKeyChecking=no ec2-user@%s '%s'" % (cred.private_key, 
                                                                          instance.public_dns_name, 
                                                                          self.command)
        ShellExecutor(self.logger).run(cmd)
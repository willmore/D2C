from d2c.ShellExecutor import ShellExecutor

class Action():

    def __init__(self, command):
        self.command = command
     
    def execute(self, instance):   
        cred = self.dao.getEC2Cred(instance.key_name)
            
        for action in self.startActions:
            action.execute(instance)
            cmd = "rsh -i %s -o StrictHostKeyChecking=no ec2-user@%s '%s'" % (cred.private_key, 
                                                                               instance.public_dns_name, 
                                                                               action.command)
            ShellExecutor(self.logger).run(cmd)
import re

class ConfValidationError(Exception):
    def __init__(self, value):
        Exception.__init__(self)
        self.value = value
        
    def __str__(self):
        return repr(self.value)

class Configuration(object):
    
    def __init__(self, awsUserId, ec2Cred, awsCred):
        #self.ec2ToolHome = ec2ToolHome
        self.awsUserId = awsUserId
        self.awsCred = awsCred
        self.ec2Cred = ec2Cred
        
    def validate(self):
        #self.__validateEc2ToolHome()
        self.__validateAwsUserId()
    
    '''    
    def __validateEc2ToolHome(self):
        if self.ec2ToolHome and not os.path.exists(self.ec2ToolHome):
            raise ConfValidationError("EC2 Tool Home directory not found: " + self.ec2ToolHome)
    '''
    
    def __validateAwsUserId(self):
        return self.awsUserId and re.match("\d{12}", self.awsUserId)
    
    def __str__(self):
        return "awsCred: %s, ec2Cred: %s" % (str(self.awsCred), str(self.ec2Cred))
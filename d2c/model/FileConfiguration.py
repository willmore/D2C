'''
Created on Apr 11, 2011

@author: willmore
'''
from d2c.model.AWSCred import AWSCred
from d2c.model.EC2Cred import EC2Cred
from d2c.model.Configuration import Configuration
import string

class FileConfiguration(Configuration):
    '''
    Configuration settings that can be loaded from a simple 
    file for testing purposes.
    '''
    
    
    def __init__(self, confFile):
        settings = {}
        for l in open(confFile, "r"):
            (k, v) = string.split(l.strip(), "=")
            settings[k] = v
     
        ec2Cred = EC2Cred(settings['ec2CredId'], 
                          settings['cert'], 
                          settings['privateKey'])
    
        awsCred = AWSCred(settings['accessKey'],
                          settings['secretKey'])
        
        Configuration.__init__(self,
                               #ec2ToolHome='/opt/EC2_TOOLS',
                               awsUserId=settings['userid'],
                               ec2Cred=ec2Cred,
                               awsCred=awsCred)
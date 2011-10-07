'''
Created on Oct 5, 2011

@author: willmore
'''


class CloudCred(object):
    
    def __init__(self, name, awsCred=None, awsUserId=None, ec2Cred=None):
        
        self.name = name
        self.awsCred = awsCred
        self.awsUserId = awsUserId
        self.ec2Cred = ec2Cred
        
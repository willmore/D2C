'''
Created on Mar 14, 2011

@author: willmore
'''

import boto.ec2

class ConnectionError(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)

class EC2ConnectionFactory:
    
    def __init__(self, accessKey, secretKey, region, logger):
        self.region = region
        self.accessKey = accessKey
        self.secretKey = secretKey
        self.logger = logger
        
    def getConnection(self):
        
        for region in boto.ec2.regions(aws_access_key_id=self.accessKey, 
                                       aws_secret_access_key=self.secretKey):
            if region.name == self.region:
                return region.connect(aws_access_key_id=self.accessKey, 
                                      aws_secret_access_key=self.secretKey)
        
        raise ConnectionError("Cannot find region " + self.region)
        
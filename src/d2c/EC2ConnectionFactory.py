'''
Created on Apr 6, 2011

@author: willmore
'''

import boto.ec2

class EC2ConnectionFactory:
    
    
    def __init__(self, accessKey, secretKey, logger):
        self.__logger = logger
        self.__accessKey = accessKey
        self.__secretKey = secretKey
        self.__ec2Conn = None
        
    def getConnection(self):
        
        #TODO worry about thread safety?
        #TODO un-hardcode
        region = "eu-west-1"
         
        if self.__ec2Conn is None:
            #TODO: add timeout - if network connection fails, this will spin forever
            self.__logger.write("Initiating connection to ec2 region '%s'..." % region)
            self.__ec2Conn = boto.ec2.connect_to_region(region, 
                                                        aws_access_key_id=self.__accessKey, 
                                                        aws_secret_access_key=self.__secretKey)
            self.__logger.write("EC2 connection established")
            
        return self.__ec2Conn
'''
Created on Apr 6, 2011

@author: willmore
'''

from boto.ec2.cloudwatch import CloudWatchConnection
from logger import StdOutLogger

class CloudWatchConnectionFactory:
    
    
    def __init__(self, credStore, logger=StdOutLogger()):
        self.__logger = logger
        self.__credStore = credStore
        self.__conn = None
        
    def getConnection(self):
        
        #TODO worry about thread safety?
        #TODO un-hardcode
        region = "eu-west-1"
         
        cred = self.__credStore.getDefaultAWSCred()
        
        self.__logger.write("Acquiring CloudWatch connection to region %s" % region)
        
        if self.__conn is None:
            self.__conn = CloudWatchConnection(host='monitoring.%s.amazonaws.com' % region,
                                               aws_access_key_id=cred.access_key_id,
                                               aws_secret_access_key=cred.secret_access_key)
        
        self.__logger.write("CloudWatch connection acquired to region %s" % region)
        
        return self.__conn
    
'''
Created on Jun 7, 2011

@author: willmore
'''


class S3Storage:

    def __init__(self, name, serviceURL):
        self.name = name
        self.serviceURL = serviceURL
    
    def getServiceURL(self):
        return self.serviceURL
        
class AWSStorage(S3Storage):
    
    def __init__(self):
        S3Storage.__init__(self, "AWS S3", "https://s3.amazonaws.com")
        
class WalrusStorage(S3Storage):
    
    def __init__(self, name, serviceURL):
        S3Storage.__init__(self, name, serviceURL)
        
    
        
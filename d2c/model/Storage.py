import os

from d2c.ShellExecutor import ShellExecutor
from d2c.model.AWSCred import AWSCred

class S3Storage:

    class BundleUploader:
        
        def __init__(self, serviceURL):
            self.serviceURL = serviceURL
            
    def __init__(self, name, serviceURL):
        self.name = name
        self.serviceURL = serviceURL
    
    def getServiceURL(self):
        return self.serviceURL
     
        
class AWSStorage(S3Storage):
    
    def __init__(self):
        S3Storage.__init__(self, "AWS S3", "https://s3.amazonaws.com")
    
    class AWSBundleUploader(S3Storage.BundleUploader):
        
        def __init__(self, serviceURL):
            S3Storage.BundleUploader.__init__(self, serviceURL)
        
        def upload(self, manifest, bucket, awsCred):
           
            assert isinstance(bucket, basestring)
            assert isinstance(manifest, basestring)
            assert isinstance(awsCred, AWSCred)
           
            UPLOAD_CMD = "ec2-upload-bundle --url %s -b %s -m %s -a %s -s %s"
        
            uploadCmd = UPLOAD_CMD % (self.serviceURL,
                                      bucket, manifest,
                                      awsCred.access_key_id,
                                      awsCred.secret_access_key)
        
            ShellExecutor().run(uploadCmd)
            
            return bucket + "/" + os.path.basename(manifest)
        
        
    def bundleUploader(self):
        return self.AWSBundleUploader(self.serviceURL)
        
class WalrusStorage(S3Storage):
    
    def __init__(self, name, serviceURL):
        S3Storage.__init__(self, name, serviceURL)
        
    class EucBundleUploader(S3Storage.BundleUploader):
        
        def __init__(self, serviceURL):
            S3Storage.BundleUploader.__init__(self, serviceURL)
            
        def upload(self, manifest, bucket, awsCred):
           
            assert isinstance(bucket, basestring)
            assert isinstance(manifest, basestring)
            assert isinstance(awsCred, AWSCred)
           
            UPLOAD_CMD = "euca-upload-bundle --url %s -b %s -m %s -a %s -s %s"
        
            uploadCmd = UPLOAD_CMD % (self.serviceURL,
                                      bucket, manifest,
                                      awsCred.access_key_id,
                                      awsCred.secret_access_key)
        
            ShellExecutor().run(uploadCmd)
            
            return bucket + "/" + os.path.basename(manifest)
                 
        
    def bundleUploader(self):
        return self.EucBundleUploader(self.serviceURL)
        
    
        
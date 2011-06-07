#import d2c.AMICreator
import string
from d2c.logger import StdOutLogger
from d2c.model.EC2Cred import EC2Cred
import tempfile
import time
from d2c.AMICreator import AMICreator
from d2c.model.Storage import AWSStorage
from d2c.model.Region import EC2Region
from d2c.model.AWSCred import AWSCred


class AMICreatorTest():

    def main(self):
         
        settings = {}
        for l in open("/home/willmore/test.conf", "r"):
            (k, v) = string.split(l.strip(), "=")
            settings[k] = v
        
        ec2Cred = EC2Cred("default", settings['cert'], settings['privateKey'])
        awsCred = AWSCred(settings['accessKey'], 
                            settings['secretKey'])
        s3Bucket = "ee.ut.cs.cloud/test/" + str(time.time())
        disk = "/home/willmore/Downloads/dsl-4.4.10-x86.vdi"
        
        userId = settings['userid']
        region = EC2Region("eu-west-1")
        s3Storage = AWSStorage()
        
        amiCreator = AMICreator(disk, 
                 ec2Cred, awsCred,
                 userId, s3Bucket,
                 region, s3Storage,
                 tempfile.mkdtemp(),
                 StdOutLogger())
        
        ami = amiCreator.createAMI()
        
        print "Ami = " + ami

if __name__ == '__main__':
    AMICreatorTest().main()




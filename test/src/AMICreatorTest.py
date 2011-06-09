#import d2c.AMICreator
import string
from d2c.model.EC2Cred import EC2Cred
import time
from d2c.AMICreator import AMICreator
from d2c.model.Storage import AWSStorage, WalrusStorage
from d2c.model.Region import EC2Region, EucRegion
from d2c.model.AWSCred import AWSCred
from d2c.data.DAO import DAO
import os

class AMICreatorTest():

    def main(self):
         
        settings = {}
        for l in open("/home/willmore/test.conf", "r"):
            (k, v) = string.split(l.strip(), "=")
            settings[k] = v
        
        SQLITE_FILE = "%s/.d2c_test/d2c_db.sqlite" % os.path.expanduser('~') 
        if os.path.exists(SQLITE_FILE):
            print "Deleting existing DB"
            os.unlink(SQLITE_FILE)
        
        dao = DAO(SQLITE_FILE)
        
        ec2Cred = EC2Cred("default", settings['cert'], settings['privateKey'])
        awsCred = AWSCred(settings['accessKey'], 
                            settings['secretKey'])
        s3Bucket = "ee.ut.cs.cloud/test/" + str(time.time())
        disk = "/home/willmore/Downloads/dsl-4.4.10-x86.vdi"
        
        userId = settings['userid']
        region = EC2Region("eu-west-1", "/opt/EC2_TOOLS/etc/ec2/amitools/cert-ec2.pem")
        s3Storage = AWSStorage()
        
        amiCreator = AMICreator(disk, 
                 ec2Cred, awsCred,
                 userId, s3Bucket,
                 region, s3Storage,
                 dao)
        
        ami = amiCreator.createAMI()
        
        print "Ami = %s" % str(ami)

    def mainSciCloud(self):
         
        settings = {}
        for l in open("/home/willmore/scicloud.conf", "r"):
            (k, v) = string.split(l.strip(), "=")
            settings[k] = v
        
        SQLITE_FILE = "%s/.d2c_test/d2c_db.sqlite" % os.path.expanduser('~') 
        if os.path.exists(SQLITE_FILE):
            print "Deleting existing DB"
            os.unlink(SQLITE_FILE)
        
        dao = DAO(SQLITE_FILE)
        
        ec2Cred = EC2Cred("default", settings['cert'], settings['privateKey'])
        awsCred = AWSCred(settings['accessKey'], 
                            settings['secretKey'])
        
        s3Bucket = "willmore-test-" + str(time.time())
        disk = "/home/willmore/Downloads/euca-ubuntu-9.04-x86_64/ubuntu.9-04.x86-64.img"
        
        userId = settings['userid']
        region = EucRegion("SciCloud", 
                           "/home/willmore/Downloads/cloud-cert.pem",
                           "http://172.17.36.21:8773/services/Eucalyptus")
        
        s3Storage = WalrusStorage("SciCloud Storage", "http://172.17.36.21:8773/services/Walrus")
        
        amiCreator = AMICreator(disk, 
                 ec2Cred, awsCred,
                 userId, s3Bucket,
                 region, s3Storage,
                 dao)
        
        ami = amiCreator.createAMI()
        
        print "Ami = %s" % str(ami)

if __name__ == '__main__':
    AMICreatorTest().mainSciCloud()




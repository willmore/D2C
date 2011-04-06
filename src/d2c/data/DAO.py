'''
Created on Feb 10, 2011

@author: willmore
'''

import time
import os
from d2c.model.SourceImage import SourceImage
from d2c.model.EC2Cred import EC2Cred
from d2c.model.AWSCred import AWSCred
from d2c.model.Configuration import Configuration
from d2c.model.AMI import AMI
from d2c.model.Deployment import Deployment
from d2c.model.Deployment import Role

import sqlite3

class DAO:
    
    _SQLITE_FILE = "~/.d2c/db.sqlite"
    
    def __init__(self):
        
        baseDir = os.path.dirname(DAO._SQLITE_FILE)
        print baseDir
        if not os.path.isdir(baseDir):
            os.makedirs(baseDir, mode=0700)
        
        
        self._conn = sqlite3.connect(DAO._SQLITE_FILE)  
        self._conn.row_factory = sqlite3.Row
        self._init_db()
        
    def _init_db(self):
        
        c = self._conn.cursor()
        
        c.execute('''create table if not exists aws_cred
                    (id integer primary key, 
                    access_key_id text, 
                    secret_access_key text)''')
        
        c.execute('''create table if not exists src_img
                    (path text primary key)''')
        
        c.execute('''create table if not exists ami
                    (id text primary key,
                    src_img text,
                    foreign key(src_img) REFERENCES src_img(path))''')
        
        c.execute('''create table if not exists ec2_cred
                    (id integer primary key, 
                    cert text, 
                    private_key text)''')
        
        c.execute('''create table if not exists conf
                    (key text, value text)''')
        
        c.execute('''create table if not exists deploy
                    (name text primary key,
                    state text
                    )''')
              
              
        c.execute('''create table if not exists deploy_role
                    (name text,
                    deploy text,
                    ami text,
                    count integer,
                    foreign key(deploy) references deploy(name),
                    foreign key(ami) references ami(id))''')
    
        c.execute('''create table if not exists ami_creation_job
                    (id integer primary key, 
                    src_img text,
                    log text,
                    start_time integer, 
                    end_time integer, 
                    return_code integer,
                    foreign key(src_img) references src_img(path))''')
        
        self._conn.commit()
        c.close()
          
    def createAmiJob(self, srcImg, startTime=time.time()):
        c = self._conn.cursor()

        c.execute("insert into ami_creation_job (src_img, start_time) values (?,?)", (srcImg,startTime))
        newId = c.lastrowid
        self._conn.commit()
        c.close()
        
        return newId
    
    def createAmi(self, amiId, srcImg):
        
        c = self._conn.cursor()
        c.execute("insert into ami (id, src_img) values (?,?)", (amiId,srcImg))
        self._conn.commit()
        c.close()
    
    def setAmiJobFinishTime(self, jobId, endTime):        
        c = self._conn.cursor()

        c.execute("update ami_creation_job set end_time=? where id=?", (endTime,jobId))
        newId = c.lastrowid
        self._conn.commit()
        c.close()
        
        return newId
    
    def setAmiJobLog(self, jobId, log):        
        c = self._conn.cursor()

        c.execute("update ami_creation_job set log=? where id=?", (log,jobId))
        newId = c.lastrowid
        self._conn.commit()
        c.close()
        
        return newId
    
    def getSourceImages(self):
        c = self._conn.cursor()

        out = []

        for r in c.execute("select path from src_img"):
            out.append(SourceImage(r[0]))
        
        c.close()
        
        return out   
    
    def addSourceImage(self, srcImgPath):
                
        c = self._conn.cursor()

        c.execute("insert into src_img values (?)", (srcImgPath,))
        
        self._conn.commit()
        c.close()
        
    def saveConfiguration(self, conf):
        c = self._conn.cursor()
        
        self.__saveConfigurationValue(c, 'ec2ToolHome', conf.ec2ToolHome)
        self.__saveConfigurationValue(c, 'awsUserId', conf.awsUserId)
        
        if conf.ec2Cred is not None:
            self.__saveConfigurationValue(c, 'ec2Cert', conf.ec2Cred.cert)
            self.__saveConfigurationValue(c, 'ec2PrivateKey', conf.ec2Cred.private_key)
  
        if conf.awsCred is not None:
            self.__saveConfigurationValue(c, 'awsAccessKeyId', conf.awsCred.access_key_id)
            self.__saveConfigurationValue(c, 'awsSecretAccessKey', conf.awsCred.secret_access_key)
        
        self._conn.commit()
        c.close()
        
    def getConfiguration(self):
        c = self._conn.cursor()
        
        ec2ToolHome = self.__getConfigurationValue(c, 'ec2ToolHome')
        awsUserId = self.__getConfigurationValue(c, 'awsUserId')
                    
        ec2Cert = self.__getConfigurationValue(c, 'ec2Cert')
        ec2PrivateKey = self.__getConfigurationValue(c, 'ec2PrivateKey')
              
        awsAccessKeyId = self.__getConfigurationValue(c, 'awsAccessKeyId')
        awsSecretAccessKey = self.__getConfigurationValue(c, 'awsSecretAccessKey')
        
        self._conn.commit()
        c.close()
        
        ec2Cred = EC2Cred(ec2Cert, ec2PrivateKey) if (ec2Cert is not None and ec2PrivateKey is not None) else None
        awsCred = AWSCred(awsAccessKeyId, awsSecretAccessKey) if (awsAccessKeyId is not None and awsSecretAccessKey is not None) else None
            
        return Configuration(ec2ToolHome=ec2ToolHome,
                             awsUserId=awsUserId,
                             ec2Cred=ec2Cred,
                             awsCred=awsCred)
        
    def __getConfigurationValue(self, cursor, key): 
        h = cursor.execute("select value from conf where key=? limit 1", (key,)) 
        
        r = h.fetchone()
        
        if r is not None:
            return r[0]
        else:
            return None
                            
        
    def __saveConfigurationValue(self, cursor, key, value):

        if None is cursor.execute("select * from conf where key==?", (key,)).fetchone():
            cursor.execute("insert into conf (key, value) values (?,?)", (key, value))
        else:
            cursor.execute("update conf set value=? where key=?", (value, key))
    
    def __rowToAMI(self, row):
        
        return AMI(amiId=row['id'], srcImg=row['src_img'])
    
    def getAMIBySrcImg(self, srcImg):
        c = self._conn.cursor()
        
        h = c.execute("select * from ami where src_img=? limit 1", (srcImg,))
        
        row = h.fetchone()
        
        c.close()
    
        return self.__rowToAMI(row) if row is not None else None
    
    def getAMIById(self, id):
        c = self._conn.cursor()
        
        h = c.execute("select * from ami where id=? limit 1", (id,))
        
        row = h.fetchone()
        
        c.close()
    
        return self.__rowToAMI(row) if row is not None else None
    
    def getAMIs(self):
        c = self._conn.cursor()
        
        c.execute("select * from ami")
        
        out = []
        
        for row in c:
            out.append(self.__rowToAMI(row))
        
        c.close()
    
        return out
    
    def addAMI(self, amiid, srcImg=None):     
        c = self._conn.cursor()

        c.execute("insert into ami(id, src_img) values (?,?)", (amiid, srcImg))
        
        self._conn.commit()
        c.close()  
    
    def saveDeployment(self, deployment):
        print "Saving new deployment"
        c = self._conn.cursor()
        c.execute("insert into deploy (name, state) values (?, ?)", 
                      (deployment.id, deployment.state))
    
        for role in deployment.roles:
            c.execute("insert into deploy_role (name, deploy, ami, count) values (?,?,?,?)", 
                  (role.name, deployment.id, role.ami.amiId, role.count))
            
        self._conn.commit()
        c.close()  
    
    def getDeployments(self):
        
        amis = {}
        for a in self.getAMIs():
            amis[a.amiId] = a
        
        c = self._conn.cursor()
        
        c.execute("select * from deploy")
        
        deploys = {}
        
        for row in c:
            deploys[row['name']] = self.rowToDeployment(row)
        
        c.execute("select * from deploy_role")
        
        for row in c:
            role = self.rowToRole(row)
            role.ami = amis[role.ami] #hack
            assert deploys.has_key(row['deploy'])
            print "Adding role to %s" %row['deploy']
            deploys[row['deploy']].addRole(role)
        
        c.close()
      
        for d in deploys.values():
            print d
        return deploys.values()
             
    def rowToDeployment(self, row):
        return Deployment(row['name'])
        
    def rowToRole(self, row):
        return Role(row['name'], row['ami'], row['count'])
    

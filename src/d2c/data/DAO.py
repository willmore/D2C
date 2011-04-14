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
from d2c.model.Role import Role

import sqlite3

class DAO:
    
    _SQLITE_FILE = "~/.d2c/db.sqlite"
    
    def __init__(self):
        
        baseDir = os.path.dirname(DAO._SQLITE_FILE)
        print baseDir
        if not os.path.isdir(baseDir):
            os.makedirs(baseDir, mode=0700)
        
        self.__conn = None
        
        self._init_db()
        
    def __getConn(self):
        if self.__conn is None:
            self.__conn = sqlite3.connect(DAO._SQLITE_FILE, check_same_thread=False)
            self.__conn.row_factory = sqlite3.Row

        return self.__conn
        
    def _init_db(self):
        
        c = self.__getConn().cursor()
        
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
                    (id string primary key, 
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
                    primary key (name, deploy)
                    foreign key(deploy) references deploy(name),
                    foreign key(ami) references ami(id))''')
        
        c.execute('''create table if not exists deploy_role_instance
                    (instance text primary key, -- AWS instance ID
                    role_name text,
                    role_deploy text,
                    foreign key(role_name, role_deploy) references deploy_role(name, deploy))''')
    
        c.execute('''create table if not exists ami_creation_job
                    (id integer primary key, 
                    src_img text,
                    log text,
                    start_time integer, 
                    end_time integer, 
                    return_code integer,
                    foreign key(src_img) references src_img(path))''')
        
        c.execute('''create table if not exists metric
                    (name text primary key,
                     unit text not null)''')
        
        c.execute('''
                    create table if not exists instance_metric
                    (instance_id text not null,
                    metric text not null,
                    time integer not null,
                    primary key (instance_id, metric, time)
                    foreign key(instance_id) references deploy_role_instance(instance))
                ''')
        
        self.__getConn().commit()
        c.close()
          
    def createAmiJob(self, srcImg, startTime=time.time()):
        c = self.__getConn().cursor()

        c.execute("insert into ami_creation_job (src_img, start_time) values (?,?)", (srcImg,startTime))
        newId = c.lastrowid
        self.__getConn().commit()
        c.close()
        
        return newId
    
    def createAmi(self, amiId, srcImg):
        
        c = self.__getConn().cursor()
        c.execute("insert into ami (id, src_img) values (?,?)", (amiId,srcImg))
        self.__getConn().commit()
        c.close()
    
    def setAmiJobFinishTime(self, jobId, endTime):        
        c = self.__getConn().cursor()

        c.execute("update ami_creation_job set end_time=? where id=?", (endTime,jobId))
        newId = c.lastrowid
        self.__getConn().commit()
        c.close()
        
        return newId
    
    def setAmiJobLog(self, jobId, log):        
        c = self.__getConn().cursor()

        c.execute("update ami_creation_job set log=? where id=?", (log,jobId))
        newId = c.lastrowid
        self.__getConn().commit()
        c.close()
        
        return newId
    
    def getSourceImages(self):
        c = self.__getConn().cursor()

        out = []

        for r in c.execute("select path from src_img"):
            out.append(SourceImage(r[0]))
        
        c.close()
        
        return out   
    
    def addSourceImage(self, srcImgPath):
                
        c = self.__getConn().cursor()

        c.execute("insert into src_img values (?)", (srcImgPath,))
        
        self.__getConn().commit()
        c.close()
        
    def saveConfiguration(self, conf):
        c = self.__getConn().cursor()
        
        self.__saveConfigurationValue(c, 'ec2ToolHome', conf.ec2ToolHome)
        self.__saveConfigurationValue(c, 'awsUserId', conf.awsUserId)
        
        if conf.ec2Cred is not None:
            self.__saveConfigurationValue(c, 'defaultEC2Cred', conf.ec2Cred.id)
            self.saveEC2Cred(conf.ec2Cred)
  
        if conf.awsCred is not None:
            self.__saveConfigurationValue(c, 'awsAccessKeyId', conf.awsCred.access_key_id)
            self.__saveConfigurationValue(c, 'awsSecretAccessKey', conf.awsCred.secret_access_key)
        
        self.__getConn().commit()
        c.close()
        
    def getConfiguration(self):
        c = self.__getConn().cursor()
        
        ec2ToolHome = self.__getConfigurationValue(c, 'ec2ToolHome')
        awsUserId = self.__getConfigurationValue(c, 'awsUserId')
              
        awsAccessKeyId = self.__getConfigurationValue(c, 'awsAccessKeyId')
        awsSecretAccessKey = self.__getConfigurationValue(c, 'awsSecretAccessKey')
        
        defEC2Cred = self.__getConfigurationValue(c, 'defaultEC2Cred')
        
        self.__getConn().commit()
        c.close()
        
        ec2Cred = self.getEC2Cred(defEC2Cred) if defEC2Cred is not None else None
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
        c = self.__getConn().cursor()
        
        h = c.execute("select * from ami where src_img=? limit 1", (srcImg,))
        
        row = h.fetchone()
        
        c.close()
    
        return self.__rowToAMI(row) if row is not None else None
    
    def getAMIById(self, id):
        c = self.__getConn().cursor()
        
        h = c.execute("select * from ami where id=? limit 1", (id,))
        
        row = h.fetchone()
        
        c.close()
    
        return self.__rowToAMI(row) if row is not None else None
    
    def getAMIs(self):
        c = self.__getConn().cursor()
        
        c.execute("select * from ami")
        
        out = []
        
        for row in c:
            out.append(self.__rowToAMI(row))
        
        c.close()
    
        return out
    
    def addAMI(self, amiid, srcImg=None):     
        c = self.__getConn().cursor()

        c.execute("insert into ami(id, src_img) values (?,?)", (amiid, srcImg))
        
        self.__getConn().commit()
        c.close()  
    
    def saveDeployment(self, deployment):
        print "Saving new deployment"
        c = self.__getConn().cursor()
        c.execute("insert into deploy (name, state) values (?, ?)", 
                      (deployment.id, deployment.state))
    
        for role in deployment.roles:
            c.execute("insert into deploy_role (name, deploy, ami, count) values (?,?,?,?)", 
                  (role.name, deployment.id, role.ami.amiId, role.count))
            
        self.__getConn().commit()
        c.close()  
    
    def addRoleInstance(self, roleDeployment, roleName, instanceId):
        c = self.__getConn().cursor()

        c.execute("insert into deploy_role_instance (instance, role_name, role_deploy) values (?,?,?)", 
                      (instanceId, roleName, roleDeployment))
        self.__getConn().commit()
        c.close()  
    
    def getDeployments(self):
        
        amis = {}
        for a in self.getAMIs():
            amis[a.amiId] = a
        
        c = self.__getConn().cursor()
        
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
        return Role(row['deploy'], row['name'], row['ami'], row['count'])
    
    def getEC2Cred(self, id):
        
        c = self.__getConn().cursor()
        
        c.execute("select * from ec2_cred where id = ? limit 1", (id,))
        
        row = c.fetchone()
        
        c.close()
        
        return EC2Cred(row['id'], row['cert'], row['private_key'])
    
    def saveEC2Cred(self, ec2Cred):
        
        c = self.__getConn().cursor()
        c.execute("insert into ec2_cred (id, cert, private_key) values (?,?,?)",
                 (ec2Cred.id, ec2Cred.cert, ec2Cred.private_key))
        c.close()
    
    
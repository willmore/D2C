import os
from d2c.model.SourceImage import SourceImage
from d2c.model.EC2Cred import EC2Cred
from d2c.model.AWSCred import AWSCred
from d2c.model.Configuration import Configuration
from d2c.model.Deployment import Deployment
from d2c.model.Role import Role
from d2c.model.InstanceType import InstanceType
from d2c.model.Region import Region
from d2c.model.Storage import WalrusStorage
from d2c.model.Cloud import Cloud
from d2c.model.Kernel import Kernel
from d2c.model.AMI import AMI
from d2c.model.Action import StartAction

from sqlalchemy import Table, Column, Integer, String, MetaData, ForeignKey, create_engine
from sqlalchemy.orm import sessionmaker, mapper, relationship

import boto

import string
import sqlite3

class DAO:
    
    def __init__(self, fileName="~/.d2c/db.sqlite", botoModule=boto):
    
        self.botoModule = botoModule
        self.fileName = fileName
        baseDir = os.path.dirname(self.fileName)

        if not os.path.isdir(baseDir):
            os.makedirs(baseDir, mode=0700)
        
        self.__conn = None
        
        self.engine = create_engine('sqlite:///' + self.fileName, echo=True)
        self.session = sessionmaker(bind=self.engine)()
        self.metadata = MetaData()
        
        self._init_db()
        
    def setCredStore(self, credStore):
        self.__credStore = credStore
        
    def __getConn(self):
        if self.__conn is None:
            self.__conn = sqlite3.connect(self.fileName, check_same_thread=False)
            self.__conn.row_factory = sqlite3.Row

        return self.__conn
        
    def _init_db(self):
        
        metadata = self.metadata
      
        awsCredTable = Table('aws_cred', metadata,
                            Column('name', String, primary_key=True),
                            Column('access_key_id', String),
                            Column('secret_access_key', String)
                            )  
      
        mapper(AWSCred, awsCredTable)
      
        cloudTable = Table('cloud', metadata,
                            Column('name', String, primary_key=True),
                            Column('service_url', String),
                            Column('storage_url', String),
                            Column('ec2cert', String)
                            )  
        
        mapper(Cloud, cloudTable, properties={
                                    'deploys': relationship(Deployment, backref='parent'),
                                    'amis': relationship(AMI, backref='parent')             
                                    })
      
        deploymentTable = Table('deploy', metadata,
                            Column('name', String, primary_key=True),
                            Column('cloud', String, ForeignKey('cloud.name')),
                            Column('aws_cred', String, ForeignKey("aws_cred.name")),
                            Column('state', String)
                            )  
        
        mapper(Deployment, deploymentTable, properties={
                                    'roles': relationship(Role),
                                    'awsCred' : relationship(AWSCred)
                                    })
        
        roleTable = Table('deploy_role', metadata,
                            Column('name', String, primary_key=True),
                            Column('deploy', String, ForeignKey('deploy.name'), primary_key=True),
                            Column('ami', String),
                            Column('count', Integer),
                            Column('instance_type', String)
                            )  
        
        mapper(Role, roleTable)
        
        startActionTable = Table('start_actions', metadata,
                            Column('id', Integer, primary_key=True),
                            Column('action', String),
                            Column('role', String),
                            Column('deploy', String)
                            )
        
        mapper(StartAction, startActionTable)
        
        srcImgTable = Table('src_img', metadata,
                            Column('path', String, primary_key=True)
                            )
        
        mapper(SourceImage, srcImgTable, properties={
                                'amis': relationship(AMI, backref='parent')}
                                )
        
        
        amiTable = Table('ami', metadata,
                            Column('id', String, primary_key=True),
                            Column('srcImg', String, ForeignKey('src_img.path')),
                            Column('cloud', String, ForeignKey('cloud.name'))
                            )
        
        mapper(AMI, amiTable)
        
        metadata.create_all(self.engine)
        
        c = self.__getConn().cursor()
        
        
        
        c.execute('''create table if not exists ec2_cred
                    (id string primary key, 
                    cert text, 
                    private_key text)''')
        
        c.execute('''create table if not exists conf
                    (key text, value text)''')
          
        c.execute('''create table if not exists deploy_role_instance
                    (instance text primary key, -- AWS instance ID
                    role_name text,
                    role_deploy text,
                    foreign key(role_name, role_deploy) references deploy_role(name, deploy))''')

        c.execute('''create table if not exists instance_type
                    (name text not null,
                     cloud text not null,
                     cpu integer not null, --mhz
                     cpu_count integer not null,
                     memory integer not null, --GB
                     disk integer not null, --GB
                     cost_per_hour integer not null, --USD cents
                     architecture string not null, --colon delimited list
                     foreign key(cloud) references cloud(name),
                     primary key(name, cloud))''')
        
        c.execute('''create table if not exists amikernel
                    (aki string not null, 
                    cloud text not null, 
                    arch string not null,
                    contents string not null,
                    primary key (aki, cloud),
                    foreign key(cloud) references cloud(name))''')
        
        c.execute('''create table if not exists image_store
                    (name string primary key, 
                    service_url text not null)''')
        
        self.__getConn().commit()
        c.close()
    
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
        awsCred = AWSCred("mainKey", awsAccessKeyId, awsSecretAccessKey) if (awsAccessKeyId is not None and awsSecretAccessKey is not None) else None
            
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
        
        return AMI(id=row['id'], srcImg=row['src_img'])
    
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
    
    def addAMI(self, ami):
        
        assert isinstance(ami, AMI)
             
        c = self.__getConn().cursor()

        c.execute("insert into ami(id, src_img, cloud) values (?,?,?)", (ami.id, ami.srcImg, ami.cloud.name))
        
        self.__getConn().commit()
        c.close()  
        
    def addAWSCred(self, awsCred):     
        self.session.add(awsCred)
        self.session.commit()
        
    def getAWSCred(self, id):
        
        c = self.__getConn().cursor()
        c.execute("select * from aws_cred where id = ? limit 1", (id,))
        row = c.fetchone()
        c.close()
        
        return AWSCred(row['id'], row['access_key_id'], row['secret_access_key']) if row is not None else None
    
    def saveDeployment(self, deployment):
        #TODO deprecate
        self.addDeployment(deployment)
        
    def addDeployment(self, deployment):
        self.session.add(deployment)
        self.session.flush()
    
    def updateDeployment(self, deployment):  
        #TODO Deprecate
        self.session.flush()
    
    def addRoleInstance(self, roleDeployment, roleName, instanceId):
        c = self.__getConn().cursor()

        c.execute("insert into deploy_role_instance (instance, role_name, role_deploy) values (?,?,?)", 
                      (instanceId, roleName, roleDeployment))
        self.__getConn().commit()
        c.close()  
    
    def getDeployments(self):
        
        amis = {}
        for a in self.getAMIs():
            amis[a.id] = a
        
        c = self.__getConn().cursor()
        
        c.execute("select * from deploy")
        
        deploys = {}
        
        for row in c:
            d = self.rowToDeployment(row)
            d.setCloud(self.getCloud(row['cloud']))
            d.awsCred = self.getAWSCred(row['aws_cred'])
            deploys[row['name']] = d

        c.execute("select * from deploy_role")
        
        for row in c:
            role = self.rowToRole(row)
            role.ami = amis[role.ami] #hack
            assert deploys.has_key(row['deploy'])
            deploys[row['deploy']].addRole(role)
        
        c.close()
      
        return deploys.values()
             
    def rowToDeployment(self, row):
        print self
        return Deployment(row['name'])
        
    def rowToRole(self, row):
        return Role(row['name'], 
                    row['ami'], row['count'], 
                    self.__instanceType(row['instance_type']))
    
    def getEC2Cred(self, id):
        
        c = self.__getConn().cursor()
        
        c.execute("select * from ec2_cred where id = ? limit 1", (id,))
        
        row = c.fetchone()
        
        c.close()
        
        return EC2Cred(row['id'], row['cert'], row['private_key'])
    
    def saveEC2Cred(self, ec2Cred):
        
        c = self.__getConn().cursor()
        
        c.execute("delete from ec2_cred where id=?", (ec2Cred.id,))
        c.execute("insert into ec2_cred (id, cert, private_key) values (?,?,?)",
                 (ec2Cred.id, ec2Cred.cert, ec2Cred.private_key))
        self.__getConn().commit()
        c.close()
  
    
    def __instanceType(self, name):
        '''
        Map string name to InstanceType enum.
        '''
        return getattr(InstanceType, string.replace(name.swapcase(), ".", "_"))
    
    def getRegions(self):
        c = self.__getConn().cursor()
        
        c.execute("select * from region")
    
        regions = [Region(row['name'], row['endpoint'], row['ec2cert']) for row in c]
        
        c.close()
        
        return regions
    
    def addRegion(self, region):
        c = self.__getConn().cursor()

        c.execute("insert into region (name, endpoint, ec2cert) values (?,?,?)", 
                      (region.getName(), region.getEndpoint(), region.getEC2Cert()))
        self.__getConn().commit()
        c.close()  
        
    def getImageStores(self):
        
        c = self.__getConn().cursor()
        
        c.execute("select * from image_store")
    
        stores = [WalrusStorage(row['name'], row['service_url']) for row in c]
        
        c.close()
        
        return stores
    
    def addImageStore(self, store):
        c = self.__getConn().cursor()

        c.execute("insert into image_store (name, service_url) values (?,?)", 
                      (store.name, store.serviceURL))
        self.__getConn().commit()
        c.close()  
        
    def saveCloud(self, cloud):
        
        self.session.add(cloud)
        self.session.flush()
            
    def getClouds(self):    
        return self.session.query(Cloud)
    
    def getCloud(self, name):       
        return self.session.query(Cloud).filter_by(name=name)     
        
    def saveKernel(self, kernel):
        
        assert isinstance(kernel, Kernel)
                
        c = self.__getConn().cursor()

        c.execute("insert into amikernel (aki, cloud, arch, contents) values (?,?,?,?)", 
                      (kernel.aki, kernel.cloudName, kernel.arch, kernel.contents))
        self.__getConn().commit()
        c.close() 

    def saveInstanceType(self, instanceType):
        
        assert isinstance(instanceType, InstanceType), "Object is %s" % str(instanceType)
                
        c = self.__getConn().cursor()

        c.execute("insert into instance_type (name, cloud, cpu, cpu_count, memory, disk, cost_per_hour, architecture) values (?,?,?,?,?,?,?,?)", 
                      (instanceType.name, instanceType.cloud.name, 
                       instanceType.cpu, instanceType.cpuCount,
                       instanceType.memory, instanceType.disk, 
                       instanceType.costPerHour, ":".join(str(x) for x in instanceType.architectures)))
        
        self.__getConn().commit()
        c.close() 
        
    def getKernels(self, cloudName):
        
        c = self.__getConn().cursor()
        
        c.execute("select * from amikernel where cloud = ?", (cloudName,))
    
        kernels = [Kernel(row['aki'], row['arch'], row['contents'], row['cloud']) for row in c]
        
        c.close()
        
        return kernels
    
    def getInstanceTypes(self, cloudName):
        
        c = self.__getConn().cursor()
        
        c.execute("select * from instance_type where cloud = ?", (cloudName,))
        
        instanceTypes = [InstanceType(row['name'], row['cpu'], 
                                      row['cpu_count'], row['memory'],
                                      row['disk'], string.split(row['architecture'], ":"), 
                                      row['cost_per_hour']) for row in c]
        
        c.close()
        
        return instanceTypes
        
import os
from d2c.model.SourceImage import SourceImage, Image, DesktopImage
from d2c.model.EC2Cred import EC2Cred
from d2c.model.AWSCred import AWSCred
from d2c.model.Configuration import Configuration
from d2c.model.Deployment import Deployment
from d2c.model.Role import Role
from d2c.model.InstanceType import InstanceType, Architecture
from d2c.model.Region import Region
from d2c.model.Cloud import Cloud
from d2c.model.Kernel import Kernel
from d2c.model.AMI import AMI
from d2c.model.Action import StartAction
from d2c.model.UploadAction import UploadAction
from d2c.model.DataCollector import DataCollector
from d2c.model.SSHCred import SSHCred
from d2c.model.FileExistsFinishedCheck import FileExistsFinishedCheck
from d2c.model.Ramdisk import Ramdisk
from d2c.RemoteShellExecutor import RemoteShellExecutorFactory
from d2c.ShellExecutor import ShellExecutorFactory
from d2c.model.DeploymentTemplate import DeploymentTemplate, RoleTemplate

from sqlalchemy import Table, Column, Integer, String, MetaData, ForeignKey, create_engine, Boolean
from sqlalchemy.orm import sessionmaker, mapper, relationship
from sqlalchemy.orm.interfaces import MapperExtension


import boto

import string
import sqlite3

class DAO:
    
    def __init__(self, fileName="~/.d2c/db.sqlite", botoModule=boto, 
                 remoteExecutorFactory=RemoteShellExecutorFactory(),
                 executorFactory=ShellExecutorFactory()):
    
        self.botoModule = botoModule
        self.remoteExecutorFactory = remoteExecutorFactory
        self.executorFactory = executorFactory
        self.fileName = fileName
        baseDir = os.path.dirname(self.fileName)

        if not os.path.isdir(baseDir):
            os.makedirs(baseDir, mode=0700)
        
        self.__conn = None
        
        self.engine = create_engine('sqlite:///' + self.fileName, 
                                    connect_args={'check_same_thread':False},
                                    echo=True)
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
                            Column('name', String, primary_key=True, nullable=False),
                            Column('serviceURL', String, nullable=False),
                            Column('storageURL', String, nullable=False),
                            Column('ec2Cert', String, nullable=False)
                            )       
        
        class CloudExtension(MapperExtension):
            
            def __init__(self, daoRef):
                MapperExtension.__init__(self)
                self.daoRef = daoRef
            
            def reconstruct_instance(self, mapper, instance):
                self.daoRef._setCloudBotoModule(instance)
        
        mapper(Cloud, cloudTable, properties={
                                    'deploys': relationship(Deployment, backref='cloud'),
                                    'instanceTypes': relationship(InstanceType, backref='cloud')},                                
                                    extension=CloudExtension(self)
                                  )
      
        deploymentTable = Table('deploy', metadata,
                            Column('id', String, primary_key=True),
                            Column('cloud_id', String, ForeignKey('cloud.name'), nullable=False),
                            Column('aws_cred_id', String, ForeignKey("aws_cred.name")),
                            Column('deployment_template_id', String, ForeignKey("deployment_template.id")),
                            Column('state', String, nullable=False),
                            Column('dataDir', String)
                            )  
        
        mapper(Deployment, deploymentTable, properties={
                                    'roles': relationship(Role, backref='deployment'),
                                    'awsCred' : relationship(AWSCred)
                                    })
        
        deploymentTemplateTable = Table('deployment_template', metadata,
                            Column('id', String, primary_key=True),
                            Column('dataDir', String)
                            )  
        
        mapper(DeploymentTemplate, deploymentTemplateTable, properties={
                                    'roleTemplates': relationship(RoleTemplate, backref='deploymentTemplate'),
                                    'deployments': relationship(Deployment, backref='deploymentTemplate'),    
                                    })
        
        sshCredTable = Table('ssh_cred', metadata,
                            Column('id', String, primary_key=True),
                            Column('username', String),
                            Column('privateKey', String)
                            )  
        
        mapper(SSHCred, sshCredTable)
        
        instanceTypeTable = Table('instance_type', metadata,
                            Column('name', String, primary_key=True),
                            Column('cloud_id', String, ForeignKey('cloud.name'), primary_key=True, nullable=False),
                            Column('cpu', Integer),
                            Column('cpuCount', Integer),
                            Column('memory', Integer),
                            Column('disk', Integer),
                            Column('costPerHour', Integer)
                            )
        
        archTable = Table('architecture', metadata,
                            Column('arch', String, primary_key=True)
                         )
        
        instanceArchTable = Table('instance_arch_assoc', metadata,
                                  Column('instance', Integer, ForeignKey('instance_type.name')),
                                  Column('arch', Integer, ForeignKey('architecture.arch')),
                                  )
        
        mapper(InstanceType, instanceTypeTable, 
               properties={'architectures': relationship(Architecture, secondary=instanceArchTable)}
               )
        
        mapper(Architecture, archTable)
        
        class ActionExtension(MapperExtension):
            
            def __init__(self, daoRef):
                self.daoRef = daoRef
            
            def reconstruct_instance(self, mapper, instance):
                self.daoRef._setRemoteShellExecutorFactory(instance)
        
        actionExtension = ActionExtension(self)
        
        startActionTable = Table('start_action', metadata,
                            Column('id', Integer, primary_key=True),
                            Column('action', String),
                            Column('role_id', ForeignKey('role_template.id')),
                            Column('deploy_id', ForeignKey('deploy.id'))
                            )
        
        mapper(StartAction, startActionTable, extension=actionExtension)
        
        startActionTable = Table('upload_action', metadata,
                            Column('id', Integer, primary_key=True),
                            Column('source', String),
                            Column('destination', String),
                            Column('role_id', ForeignKey('role_template.id')),
                            Column('deployment_template_id', ForeignKey('deployment_template.id')),
                            Column('ssh_cred_id', String, ForeignKey('ssh_cred.id'))
                            )
        
        mapper(UploadAction, startActionTable, properties={
                'sshCred': relationship(SSHCred)}, extension=actionExtension)
        
        
        dataCollectorTable = Table('data_collector', metadata,
                            Column('id', Integer, primary_key=True),
                            Column('source', String),
                            Column('role_id', ForeignKey('role_template.id')),
                            Column('deployment_template_id', ForeignKey('deployment_template.id')),
                            Column('ssh_cred_id', String, ForeignKey('ssh_cred.id'))
                            )
        
        mapper(DataCollector, dataCollectorTable, properties={
                'sshCred': relationship(SSHCred)}, 
                extension=actionExtension)
        
        finishedCheckTable = Table('finished_check', metadata,
                            Column('id', Integer, primary_key=True),
                            Column('fileName', String),
                            Column('role_template_id', ForeignKey('role_template.id')),
                            Column('deployment_template_id', ForeignKey('deployment_template.id'))
                            )
        
        mapper(FileExistsFinishedCheck, finishedCheckTable, extension=actionExtension)
        
          
        srcImgTable = Table('src_img', metadata,
                            Column('id', Integer, primary_key=True),
                            Column('image_id', ForeignKey('image.id')),
                            Column('type', String(30), nullable=False)
                            )
            
        imgTable = Table('image', metadata,
                            Column('id', Integer, primary_key=True),
                            Column('name', String, unique=True),
                            Column('original_id', Integer, ForeignKey('src_img.id', use_alter=True, name='fk_orig_constraint'))
                            )
        
        desktopImageTable = Table('desktop_img', metadata,
                            Column('path', String),
                            Column('id', Integer, ForeignKey('src_img.id'), primary_key=True)
                            )
            
        amiTable = Table('ami', metadata,
                            Column('id', Integer, ForeignKey('src_img.id'), primary_key=True),
                            Column('amiId', String),
                            Column('cloud_id', String, ForeignKey('cloud.name'), nullable=False, primary_key=True),
                            Column('kernel_id', String, ForeignKey('kernel.aki'), nullable=False),
                            Column('ramdisk_id', String, ForeignKey('ramdisk.id'), nullable=True),
                            )
        
        
        
        ramdiskTable = Table('ramdisk', metadata,
                             Column('id', String, primary_key=True),
                             Column('cloud_id', String, ForeignKey('cloud.name'), nullable=False),
                             Column('arch_id', String, ForeignKey('architecture.arch'), nullable=False),
                            )
        
        mapper(Ramdisk, ramdiskTable, properties={'cloud':relationship(Cloud, backref='ramdisks'),
                                                'architecture':relationship(Architecture),
                                                })
        
        kernelTable = Table('kernel', metadata,
                            Column('aki', String, primary_key=True),
                            Column('contents', String),
                            Column('cloud_id', String, ForeignKey('cloud.name'), nullable=False),
                            Column('arch_id', String, ForeignKey('architecture.arch'), nullable=False),
                            Column('ramdisk_id', String, ForeignKey('ramdisk.id'), nullable=True),
                            Column('isPvGrub', Boolean, nullable=False)
                            )
        
        mapper(Kernel, kernelTable, properties={'cloud':relationship(Cloud, backref='kernels'),
                                                'architecture':relationship(Architecture),
                                               'recommendedRamdisk':relationship(Ramdisk) })
        
        roleTemplateTable = Table('role_template', metadata,
                            Column('id', String, primary_key=True),
                            Column('deployment_template_id', String, ForeignKey('deployment_template.id'), primary_key=True),
                            Column('context_cred_id', String, ForeignKey('ssh_cred.id')),
                            Column('launch_cred_id', String, ForeignKey('ssh_cred.id')),
                            Column('image_id', Integer, ForeignKey('image.id'), nullable=False)
                            )  
        
        mapper(RoleTemplate, roleTemplateTable, properties={
                                    'image' : relationship(Image),
                                    'startActions': relationship(StartAction),
                                    'uploadActions': relationship(UploadAction),
                                    'dataCollectors': relationship(DataCollector),
                                    'finishedChecks': relationship(FileExistsFinishedCheck),
                                    'contextCred': relationship(SSHCred, primaryjoin=roleTemplateTable.c.context_cred_id==sshCredTable.c.id),
                                    'launchCred': relationship(SSHCred, primaryjoin=roleTemplateTable.c.launch_cred_id==sshCredTable.c.id)
                                    }, extension=actionExtension)
        
        roleTable = Table('role', metadata,
                            Column('id', String, primary_key=True),
                            Column('deploy_id', String, ForeignKey('deploy.id'), primary_key=True),
                            Column('src_img_id', Integer, ForeignKey('src_img.id')),
                            Column('count', Integer),
                            Column('pollRate', Integer),
                            Column('instance_type_id', String, ForeignKey('instance_type.name'))
                            )  
        
        mapper(Role, roleTable, properties={
                                    'instanceType': relationship(InstanceType),
                                    'sourceImage' : relationship(SourceImage)
                                     }, extension=actionExtension)
        
        mapper(SourceImage, srcImgTable, polymorphic_on=srcImgTable.c.type, polymorphic_identity='src_img')
        
        mapper(Image, imgTable, 
               properties={'images': relationship(SourceImage, backref='image', primaryjoin=srcImgTable.c.image_id==imgTable.c.id),
                           'originalImage': relationship(SourceImage, post_update=True, primaryjoin=imgTable.c.original_id==srcImgTable.c.id)
                           })
        
        mapper(DesktopImage, desktopImageTable, inherits=SourceImage, polymorphic_identity='desktop_img')
        
        mapper(AMI, amiTable, 
                    inherits=SourceImage, polymorphic_identity='ami_img',
                    properties={
                        'cloud': relationship(Cloud),
                        'kernel': relationship(Kernel),
                        'ramdisk': relationship(Ramdisk)
                    })
        
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
                    foreign key(role_name, role_deploy) references role(name, deploy))''')
    
        self.__getConn().commit()
        c.close()
    
    def _setRemoteShellExecutorFactory(self, action):
        action.remoteExecutorFactory = self.remoteExecutorFactory
        action.executorFactory = self.executorFactory
    
    def _setCloudBotoModule(self, cloud):
        assert isinstance(cloud, Cloud)
        cloud.botoModule = self.botoModule
 
    def add(self, entity):   
        self.session.add(entity)
        self.session.commit()
        
    def save(self, _):
        self.session.commit()
        
    def getArchitectures(self):
        return self.session.query(Architecture)
    
    def getArchitecture(self, id):
        return self.session.query(Architecture).filter_by(arch=id).first()
    
    def getSourceImages(self):
        return self.session.query(SourceImage)
    
    def getDesktopImages(self):
        return self.session.query(DesktopImage)
        
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
    
    def getAMIs(self):
        return self.session.query(AMI)
    
    def addAMI(self, ami):
        
        self.session.add(ami)
        self.session.commit()
       
    def addAWSCred(self, awsCred):     
        self.session.add(awsCred)
        self.session.commit()
        
    def getAWSCred(self, id):
        
        c = self.__getConn().cursor()
        c.execute("select * from aws_cred where id = ? limit 1", (id,))
        row = c.fetchone()
        c.close()
        
        return AWSCred(row['id'], row['access_key_id'], row['secret_access_key']) if row is not None else None
    
    def addRoleInstance(self, roleDeployment, roleName, instanceId):
        c = self.__getConn().cursor()

        c.execute("insert into deploy_role_instance (instance, role_name, role_deploy) values (?,?,?)", 
                      (instanceId, roleName, roleDeployment))
        self.__getConn().commit()
        c.close()  
    
    def getDeployments(self):
        return self.session.query(Deployment)
    
    def getDeploymentTemplates(self):
        return self.session.query(DeploymentTemplate)
             
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
            
    def getClouds(self):    
        return self.session.query(Cloud).all()
    
    def getCloud(self, name):       
        return self.session.query(Cloud).filter_by(name=name).one()     

    def addInstanceType(self, instanceType):
        self.session.add(instanceType)
        
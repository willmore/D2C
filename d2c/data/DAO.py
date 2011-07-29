import os
from d2c.model.SourceImage import SourceImage, Image, DesktopImage, AMI
from d2c.model.EC2Cred import EC2Cred
from d2c.model.AWSCred import AWSCred
from d2c.model.Configuration import Configuration
from d2c.model.Deployment import Deployment, StateEvent
from d2c.model.Role import Role
from d2c.model.InstanceType import InstanceType, Architecture
from d2c.model.Region import Region
from d2c.model.Cloud import EC2Cloud, DesktopCloud, Cloud
from d2c.model.Kernel import Kernel
from d2c.model.Action import StartAction
from d2c.model.UploadAction import UploadAction
from d2c.model.DataCollector import DataCollector
from d2c.model.SSHCred import SSHCred
from d2c.model.FileExistsFinishedCheck import FileExistsFinishedCheck
from d2c.model.Ramdisk import Ramdisk
from d2c.RemoteShellExecutor import RemoteShellExecutorFactory
from d2c.ShellExecutor import ShellExecutorFactory
from d2c.model.DeploymentTemplate import DeploymentTemplate, RoleTemplate

from sqlalchemy import Table, Column, Integer, String, MetaData, ForeignKey, create_engine, Boolean, UniqueConstraint
from sqlalchemy.orm import sessionmaker, mapper, relationship
from sqlalchemy.orm.interfaces import MapperExtension


import boto

import string

class ConfValue(object):
    
    def __init__(self, key, value):
        self.key = key
        self.value = value

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
        
        self.__credStore = None
        
        self.engine = create_engine('sqlite:///' + self.fileName, 
                                    connect_args={'check_same_thread':False},
                                    echo=True)
        self.session = sessionmaker(bind=self.engine)()
        self.metadata = MetaData()
        
        self._init_db()

    def setCredStore(self, credStore):
        self.__credStore = credStore
        
    def _init_db(self):
        
        metadata = self.metadata
      
        awsCredTable = Table('aws_cred', metadata,
                            Column('name', String, primary_key=True),
                            Column('access_key_id', String),
                            Column('secret_access_key', String)
                            )  
      
        mapper(AWSCred, awsCredTable)
      
        cloudTable = Table('cloud', metadata,
                            Column('id', Integer, primary_key=True),
                            Column('name', String),
                            Column('type', String, nullable=False)
                            )
        
        ec2CloudTable  = Table('ec2cloud', metadata,
                            Column('name', ForeignKey("cloud.id"), primary_key=True),
                            Column('serviceURL', String, nullable=False),
                            Column('storageURL', String, nullable=False),
                            Column('ec2Cert', String, nullable=False),
                        )
        
        class CloudExtension(MapperExtension):
            
            def __init__(self, daoRef):
                MapperExtension.__init__(self)
                self.daoRef = daoRef
            
            def reconstruct_instance(self, _, instance):
                self.daoRef.setCloudBotoModule(instance)
        
        mapper(Cloud, cloudTable, properties={
                                    'deploys': relationship(Deployment, backref='cloud'),
                                    'instanceTypes': relationship(InstanceType, backref='cloud')},                                
                                    extension=CloudExtension(self),
                                    polymorphic_on=cloudTable.c.type, polymorphic_identity='cloud')
        
        mapper(EC2Cloud, ec2CloudTable, inherits=Cloud, polymorphic_identity='ec2')
        
        mapper(DesktopCloud, cloudTable, inherits=Cloud, polymorphic_identity='desktop')
      
        stateEventTable = Table('state_event', metadata,
                                Column('deployment_id', ForeignKey('deploy.id'), primary_key=True),
                                Column('state', String, primary_key=True),
                                Column('time', Integer, nullable=False)
                                )
        
        mapper(StateEvent, stateEventTable, properties={
                                    'deployment': relationship(Deployment, backref='stateEvents')
                                    })
        
        deploymentTable = Table('deploy', metadata,
                            Column('id', String, primary_key=True),
                            Column('cloud_id', ForeignKey('cloud.id'), nullable=False),
                            Column('aws_cred_id', String, ForeignKey("aws_cred.name")),
                            Column('deployment_template_id', String, ForeignKey("deployment_template.id")),
                            Column('state', String, nullable=False),
                            Column('dataDir', String),
                            Column('pollRate', Integer)
                            )  
        
        mapper(Deployment, deploymentTable, properties={
                                    'roles': relationship(Role, backref='deployment'),
                                    'awsCred' : relationship(AWSCred)
                                    })
        
        deploymentTemplateTable = Table('deployment_template', metadata,
                            Column('id', Integer, primary_key=True),
                            Column('name', String, unique=True),
                            Column('dataDir', String)
                            )  
        
        mapper(DeploymentTemplate, deploymentTemplateTable, properties={
                                    'roleTemplates': relationship(RoleTemplate, backref='deploymentTemplate'),
                                    'deployments': relationship(Deployment, backref='deploymentTemplate'),    
                                    })
        
        sshCredTable = Table('ssh_cred', metadata,
                            Column('id', Integer, primary_key=True),
                            Column('name', String),
                            Column('username', String),
                            Column('privateKey', String)
                            )  
        
        mapper(SSHCred, sshCredTable)
        
        instanceTypeTable = Table('instance_type', metadata,
                            Column('id', Integer, primary_key=True),
                            Column('name', String),
                            Column('cloud_id', ForeignKey('cloud.id'), nullable=False),
                            Column('cpu', Integer),
                            Column('cpuCount', Integer),
                            Column('memory', Integer),
                            Column('disk', Integer),
                            Column('costPerHour', Integer),
                            
                            UniqueConstraint('name', 'cloud_id')
                            )
        
        archTable = Table('architecture', metadata,
                            Column('arch', String, primary_key=True)
                         )
        
        instanceArchTable = Table('instance_arch_assoc', metadata,
                                  Column('instance', ForeignKey('instance_type.id')),
                                  Column('arch', ForeignKey('architecture.arch')),
                                  )
        
        mapper(InstanceType, instanceTypeTable, 
               properties={'architectures': relationship(Architecture, secondary=instanceArchTable)}
               )
        
        mapper(Architecture, archTable)
        
        class ActionExtension(MapperExtension):
            
            def __init__(self, daoRef):
                MapperExtension.__init__(self)
                self.daoRef = daoRef
            
            def reconstruct_instance(self, _, instance):
                self.daoRef.setRemoteShellExecutorFactory(instance)
        
        actionExtension = ActionExtension(self)
        
        startActionTable = Table('start_action', metadata,
                            Column('id', Integer, primary_key=True),
                            Column('command', String),
                            Column('role_template_id', ForeignKey('role_template.id')),
                            Column('role_id', ForeignKey('role.id')),
                            Column('deploy_id', ForeignKey('deploy.id'))
                            )
        
        mapper(StartAction, startActionTable, extension=actionExtension)
        
        uploadActionTable = Table('upload_action', metadata,
                            Column('id', Integer, primary_key=True),
                            Column('source', String),
                            Column('destination', String),
                            Column('role_template_id', ForeignKey('role_template.id')),
                            Column('role_id', ForeignKey('role.id')),
                            Column('deployment_template_id', ForeignKey('deployment_template.id')),
                            Column('ssh_cred_id', ForeignKey('ssh_cred.id'))
                            )
        
        mapper(UploadAction, uploadActionTable, properties={
                'sshCred': relationship(SSHCred)}, extension=actionExtension)
        
        
        dataCollectorTable = Table('data_collector', metadata,
                            Column('id', Integer, primary_key=True),
                            Column('source', String),
                            Column('role_template_id', ForeignKey('role_template.id')),
                            Column('role_id', ForeignKey('role.id')),
                            Column('deployment_template_id', ForeignKey('deployment_template.id')),
                            Column('ssh_cred_id', ForeignKey('ssh_cred.id'))
                            )
        
        mapper(DataCollector, dataCollectorTable, properties={
                'sshCred': relationship(SSHCred)}, 
                extension=actionExtension)
        
        finishedCheckTable = Table('finished_check', metadata,
                            Column('id', Integer, primary_key=True),
                            Column('fileName', String),
                            Column('role_template_id', ForeignKey('role_template.id')),
                            Column('role_id', ForeignKey('role.id')),
                            Column('deployment_template_id', ForeignKey('deployment_template.id')),
                            Column('ssh_cred_id', ForeignKey('ssh_cred.id'))
                            )
        
        mapper(FileExistsFinishedCheck, finishedCheckTable, properties={'sshCred': relationship(SSHCred)}, extension=actionExtension)
          
        srcImgTable = Table('src_img', metadata,
                            Column('id', Integer, primary_key=True),
                            Column('image_id', ForeignKey('image.id')),
                            Column('type', String(30), nullable=False),
                            Column('cloud_id', ForeignKey('cloud.id'), nullable=False)
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
                            Column('amiId', String, nullable=False),
                            Column('kernel_id', String, ForeignKey('kernel.aki'), nullable=True),
                            Column('ramdisk_id', String, ForeignKey('ramdisk.id'), nullable=True),
                        )
        
        ramdiskTable = Table('ramdisk', metadata,
                             Column('id', String, primary_key=True),
                             Column('cloud_id', ForeignKey('cloud.id'), nullable=False),
                             Column('arch_id', ForeignKey('architecture.arch'), nullable=False),
                            )
        
        mapper(Ramdisk, ramdiskTable, properties={'cloud':relationship(Cloud, backref='ramdisks'),
                                                'architecture':relationship(Architecture),
                                                })
        
        kernelTable = Table('kernel', metadata,
                            Column('aki', String, primary_key=True),
                            Column('contents', String),
                            Column('cloud_id', ForeignKey('cloud.id'), nullable=False),
                            Column('arch_id', String, ForeignKey('architecture.arch'), nullable=False),
                            Column('ramdisk_id', String, ForeignKey('ramdisk.id'), nullable=True),
                            Column('isPvGrub', Boolean, nullable=False)
                            )
        
        mapper(Kernel, kernelTable, properties={'cloud':relationship(Cloud, backref='kernels'),
                                                'architecture':relationship(Architecture),
                                               'recommendedRamdisk':relationship(Ramdisk) })
        
        roleTemplateTable = Table('role_template', metadata,
                            Column('id', Integer, primary_key=True),
                            Column('name', String),
                            Column('deployment_template_id', String, ForeignKey('deployment_template.id')),
                            Column('context_cred_id', ForeignKey('ssh_cred.id')),
                            Column('launch_cred_id', ForeignKey('ssh_cred.id')),
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
                            Column('id', Integer, primary_key=True),
                            Column('deploy_id', ForeignKey('deploy.id'), nullable=False),
                            Column('src_img_id', Integer, ForeignKey('src_img.id'), nullable=False),
                            Column('template_id', ForeignKey('role_template.id'), nullable=False),
                            Column('count', Integer),
                            Column('pollRate', Integer),
                            Column('instance_type_id', ForeignKey('instance_type.id'))
                            )  
        
        mapper(Role, roleTable, properties={
                                    'instanceType': relationship(InstanceType),
                                    'image' : relationship(SourceImage),
                                    'template' : relationship(RoleTemplate),
                                    'startActions': relationship(StartAction),
                                    'uploadActions': relationship(UploadAction),
                                    'dataCollectors': relationship(DataCollector),
                                    'finishedChecks': relationship(FileExistsFinishedCheck)
                                     }, extension=actionExtension)
        
        mapper(SourceImage, srcImgTable, polymorphic_on=srcImgTable.c.type, polymorphic_identity='src_img', 
                    properties={
                        'cloud': relationship(Cloud)
                    })
        
        mapper(Image, imgTable, 
               properties={'reals': relationship(SourceImage, backref='image', primaryjoin=srcImgTable.c.image_id==imgTable.c.id),
                           'originalImage': relationship(SourceImage, post_update=True, primaryjoin=imgTable.c.original_id==srcImgTable.c.id)
                           })
        
        mapper(DesktopImage, desktopImageTable, inherits=SourceImage, polymorphic_identity='desktop_img')
        
        mapper(AMI, amiTable, 
                    inherits=SourceImage, 
                    polymorphic_identity='ami_img',
                    properties={
                        'kernel': relationship(Kernel),
                        'ramdisk': relationship(Ramdisk)
                    })
        
        
                
        ec2CredTable = Table('ec2_cred', metadata,
                            Column('id', String, primary_key=True),
                            Column('cert', String),
                            Column('private_key', String)
                            )  
      
        mapper(EC2Cred, ec2CredTable)
        
        confValueTable = Table('conf', metadata,
                            Column('key', String, primary_key=True),
                            Column('value', String))  
      
        mapper(ConfValue, confValueTable)
        
        metadata.create_all(self.engine)
        '''  
        c.execute(create table if not exists deploy_role_instance
                    (instance text primary key, -- AWS instance ID
                    role_name text,
                    role_deploy text,
                    foreign key(role_name, role_deploy) references role(name, deploy)))
        '''
    
    def setRemoteShellExecutorFactory(self, action):
        action.remoteExecutorFactory = self.remoteExecutorFactory
        action.executorFactory = self.executorFactory
    
    def setCloudBotoModule(self, cloud):
        assert isinstance(cloud, Cloud)
        cloud.botoModule = self.botoModule
 
    def add(self, entity):   
        self.session.add(entity)
        self.session.commit()
        
    def save(self, _):
        self.session.commit()
        
    def delete(self, entity):
        self.session.delete(entity)
    
    def commit(self):
        self.session.commit()
        
    def getArchitectures(self):
        return self.session.query(Architecture)
    
    def getArchitecture(self, _id):
        return self.session.query(Architecture).filter_by(arch=_id).first()
    
    def getImages(self):
        return self.session.query(Image)
    
    def getSourceImages(self):
        return self.session.query(SourceImage)
    
    def getDesktopImages(self):
        return self.session.query(DesktopImage)
        
    def saveConfiguration(self, conf):
        
        self.setConfValue('ec2ToolHome', conf.ec2ToolHome)
        self.setConfValue('awsUserId', conf.awsUserId)
        
        if conf.ec2Cred is not None:
            self.setConfValue('defaultEC2Cred', conf.ec2Cred.id)
            self.add(conf.ec2Cred)
  
        if conf.awsCred is not None:
            self.addAWSCred(conf.awsCred)
        
    def getConfiguration(self):
        
        ec2ToolHome = self.getConfValue('ec2ToolHome')
        awsUserId = self.getConfValue('awsUserId')

        
        defEC2Cred = self.getConfValue('defaultEC2Cred')
        
        ec2Cred = self.getEC2Cred(defEC2Cred) if defEC2Cred is not None else None
        awsCred = self.getAWSCred("mainKey")
            
        return Configuration(ec2ToolHome=ec2ToolHome,
                             awsUserId=awsUserId,
                             ec2Cred=ec2Cred,
                             awsCred=awsCred)
    
    def getConfValue(self, key):
        val = self.session.query(ConfValue).filter_by(key=key).scalar()
        if val is None:
            return None
        else:
            return val.value
    
    def setConfValue(self, key, value):
        
        val = self.session.query(ConfValue).filter_by(key=key).scalar()
        if val is None:
            val = ConfValue(key, value)
            self.add(val)
        else:
            val.value = value
            self.save(val)
    
    def getAMIs(self):
        return self.session.query(AMI)
    
    def addAMI(self, ami):
        self.session.add(ami)
        self.session.commit()
       
    def addAWSCred(self, awsCred):     
        self.session.add(awsCred)
        self.session.commit()
        
    def getAWSCred(self, name):
        return self.session.query(AWSCred).filter_by(name=name).one()
    
    def getDeployments(self):
        return self.session.query(Deployment)
    
    def getDeploymentTemplates(self):
        return self.session.query(DeploymentTemplate)
             
    def getEC2Cred(self, _id):
        return self.session.query(EC2Cred).filter_by(id=_id).one()
    
    def __instanceType(self, name):
        '''
        Map string name to InstanceType enum.
        '''
        return getattr(InstanceType, string.replace(name.swapcase(), ".", "_"))
            
    def getClouds(self):    
        return self.session.query(Cloud).all()
    
    def getCloud(self, name):       
        return self.session.query(Cloud).filter_by(name=name).one()     

    def addInstanceType(self, instanceType):
        self.session.add(instanceType)
        
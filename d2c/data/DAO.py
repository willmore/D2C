import time
import os
from d2c.model.SourceImage import SourceImage
from d2c.model.EC2Cred import EC2Cred
from d2c.model.AWSCred import AWSCred
from d2c.model.Configuration import Configuration
from d2c.model.Deployment import Deployment
from d2c.model.Role import Role
from d2c.data.InstanceMetrics import InstanceMetrics, Metric, MetricList, MetricValue
from d2c.model.InstanceType import InstanceType
from d2c.model.Region import Region
from d2c.model.Storage import WalrusStorage
from d2c.model.Cloud import Cloud
from d2c.model.Kernel import Kernel
from d2c.model.AMI import AMI
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
        
        self._init_db()
        
    def setCredStore(self, credStore):
        self.__credStore = credStore
        
    def __getConn(self):
        if self.__conn is None:
            self.__conn = sqlite3.connect(self.fileName, check_same_thread=False)
            self.__conn.row_factory = sqlite3.Row

        return self.__conn
        
    def _init_db(self):
        
        c = self.__getConn().cursor()
        
        c.execute('''create table if not exists aws_cred
                    (id text primary key, 
                    access_key_id text not null, 
                    secret_access_key text not null)''')
        
        c.execute('''create table if not exists src_img
                    (path text primary key)''')
        
        c.execute('''create table if not exists ami
                    (id text primary key,
                    src_img text,
                    cloud text not null,
                    foreign key(cloud) references cloud(name),
                    foreign key(src_img) REFERENCES src_img(path))''')
        
        c.execute('''create table if not exists ec2_cred
                    (id string primary key, 
                    cert text, 
                    private_key text)''')
        
        c.execute('''create table if not exists conf
                    (key text, value text)''')
        
        c.execute('''create table if not exists deploy
                    (name text primary key,
                    cloud text not null,
                    aws_cred text not null,
                    state text,
                    foreign key(cloud) references cloud(name),
                    foreign key(aws_cred) references awc_cred(id))''')
                     
        c.execute('''create table if not exists deploy_role
                    (name text,
                    deploy text,
                    ami text,
                    count integer,
                    instance_type text,
                    primary key (name, deploy)
                    foreign key(deploy) references deploy(name),
                    foreign key(ami) references ami(id),
                    foreign key(instance_type) references instance_type(name))''')
        
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
                    value float not null,
                    primary key (instance_id, metric, time)
                    foreign key(instance_id) references deploy_role_instance(instance))''')
        
        
        c.execute('''create table if not exists region
                    (name string primary key, 
                    endpoint text not null, 
                    ec2cert text not null)''')
        
        c.execute('''create table if not exists cloud
                    (name string primary key, 
                    service_url text not null, 
                    storage_url text not null,
                    ec2cert text not null)''')
        
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
        
        for name, unit in [('CPUUtilization', 'Percent'),
                           ("NetworkIn", "Bytes"),
                           ("NetworkOut", "Bytes"),
                           ("DiskWriteBytes", "Bytes"),
                           ("DiskReadBytes", "Bytes"),
                           ("DiskReadOps", "Count"),
                           ("DiskWriteOps", "Count")]:
            c.execute("insert or replace into metric (name, unit) values (?,?)",
                      (name, unit))
        
        self.__getConn().commit()
        c.close()
          
    def createAmiJob(self, srcImg, startTime=time.time()):
        c = self.__getConn().cursor()

        c.execute("insert into ami_creation_job (src_img, start_time) values (?,?)", (srcImg,startTime))
        newId = c.lastrowid
        self.__getConn().commit()
        c.close()
        
        return newId
    
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
        c = self.__getConn().cursor()
        c.execute("insert into aws_cred(id, access_key_id, secret_access_key) values (?,?,?)", (awsCred.name, awsCred.access_key_id, awsCred.secret_access_key))
        self.__getConn().commit()
        c.close()  
        
    def getAWSCred(self, id):
        
        c = self.__getConn().cursor()
        c.execute("select * from aws_cred where id = ? limit 1", (id,))
        row = c.fetchone()
        c.close()
        
        return AWSCred(row['id'], row['access_key_id'], row['secret_access_key']) if row is not None else None
    
    def saveDeployment(self, deployment):
        c = self.__getConn().cursor()
        
        c.execute("select count(*) from deploy where name=?", (deployment.id,))
        row = c.fetchone()
        c.close()
                
        if row[0] == 0:
            self.addDeployment(deployment)
        else:
            self.updateDeployment(deployment)     
        
    def addDeployment(self, deployment):
        c = self.__getConn().cursor()
        
        c.execute("insert into deploy (name, state, cloud, aws_cred) values (?,?,?,?)", 
                      (deployment.id, deployment.state, deployment.cloud.name, deployment.awsCred.name))
    
        for role in deployment.roles:
            c.execute("insert into deploy_role (name, deploy, ami, count, instance_type) values (?,?,?,?,?)", 
                  (role.name, deployment.id, role.ami.id, role.count, role.instanceType.name))
            
        self.__getConn().commit()
        c.close()
    
    def updateDeployment(self, deployment):  
        #Only updates state field now. TODO update rest of fields.
        
        c = self.__getConn().cursor()
        
        c.execute("update deploy set state = ? where name = ?", 
                      (deployment.state, deployment.id))
        
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
        
    def saveInstanceMetrics(self, instanceMetrics):
        assert isinstance(instanceMetrics, InstanceMetrics)
        
        c = self.__getConn().cursor()
                
        for mList in instanceMetrics.metricLists:
            for v in mList.values:
                c.execute("insert into instance_metric (instance_id, metric, time, value)" +
                          "values (?,?,?,?)",
                          (mList.instanceId, mList.metric.name,
                          v.time, v.value))
        
        self.__getConn().commit()
        
        c.close()
    
    def getMetrics(self):
        
        c = self.__getConn().cursor()
        
        c.execute("select * from metric")
        
        out = []
        
        for row in c:
            out.append(Metric(row['name'], row['unit']))
        
        c.close()
        
        return out
        
    def getInstanceMetrics(self, instanceId):
        
        metrics = self.getMetrics()
        
        c = self.__getConn().cursor()
        
        c.execute('''select * from instance_metric
                     where instance_id = ?''', 
                           (instanceId,))
        
        mLists = {}
        mMap = {}
        
        for m in metrics:
            mLists[m.name] = []
            mMap[m.name] = m
        
        for row in c:
            mLists[row['metric']].append(MetricValue(row['value'], row['time']))
            
        c.close()
        
        lists = []
        for mName, metrics in mLists.iteritems():
            lists.append(MetricList(instanceId, mMap[mName], metrics))
            
        return InstanceMetrics(instanceId, lists)
    
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
        
        assert isinstance(cloud, Cloud)
        
        c = self.__getConn().cursor()

        c.execute("insert into cloud (name, service_url, storage_url, ec2cert) values (?,?,?,?)", 
                      (cloud.name, cloud.serviceURL, cloud.storageURL, cloud.ec2Cert))
        self.__getConn().commit()
        c.close() 
        
        for k in cloud.kernels:
            self.saveKernel(k)
            
        for i in cloud.instanceTypes:
            self.saveInstanceType(i)
            
    def getClouds(self):
        
        c = self.__getConn().cursor()
        
        c.execute("select * from cloud")
    
        clouds = [self.__mapCloud(row) for row in c]

        c.close()
        
        for c in clouds:
            c.addInstanceTypes(self.getInstanceTypes(c.name))
        
        return clouds
    
    def getCloud(self, name):
        
        c = self.__getConn().cursor()
        
        c.execute("select * from cloud where name = ?", (name,))
    
        clouds = [self.__mapCloud(row) for row in c]
        
        c.close()
        
        for c in clouds:
            c.addInstanceTypes(self.getInstanceTypes(c.name))
        
        return clouds[0]
    
    def __mapCloud(self, row):
        return Cloud(row['name'], row['service_url'], row['storage_url'], row['ec2cert'], self.botoModule)
        
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
        
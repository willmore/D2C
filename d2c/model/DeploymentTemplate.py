import os
import random
from d2c.model.Deployment import Deployment
from d2c.model.Role import Role
from d2c.model.Cloud import Cloud

class DeploymentTemplate(object):
    
    def __init__(self, id, name, dataDir, roleTemplates, deployments=()):
        self.id = id
        self.name = name
        self.dataDir = dataDir
        self.roleTemplates = roleTemplates
        self.deployments = list(deployments)
        
    def createDeployment(self, cloud, roleCounts):
        
        assert isinstance(cloud, Cloud)
        assert len(roleCounts) == len(self.roleTemplates)
        
        roles = []
        
        dataDir = os.path.join(self.dataDir, str(random.randint(1,1000)))
        
        deployment = Deployment(None, dataDir, cloud=cloud, deploymentTemplate=self)
        
        for roleTemp, (image, instanceType, count) in roleCounts.iteritems():
            assert roleTemp in self.roleTemplates
            roles.append(roleTemp.createRole(deployment, cloud, image, instanceType, count))
        
        self.deployments.append(deployment)
        
        return deployment
        
        
class RoleTemplate(object):
    
    def __init__(self, id, name, image, roles=(), 
                 startActions=(), uploadActions=(),
                 stopActions=(), finishedChecks=(), 
                 dataCollectors=(), launchCred=None, contextCred=None):
        self.id = id
        self.name = name
        self.image = image
        self.roles = list(roles)
        self.startActions= list(startActions)
        self.uploadActions= list(uploadActions)
        self.stopActions= list(stopActions)
        self.finishedChecks= list(finishedChecks)
        self.dataCollectors= list(dataCollectors)
        self.launchCred = launchCred
        self.contextCred = contextCred
        
    def createRole(self, deployment, cloud, image, instanceType, count):
        
        return Role(None, image=image, count=count, instanceType=instanceType, 
                    roleTemplate=self, deployment=deployment)
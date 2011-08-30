import os
import random
from .Deployment import Deployment
from .Role import Role
from .Cloud import Cloud

import string

class DeploymentTemplate(object):
    
    def __init__(self, id, name, dataDir, roleTemplates, deployments=()):
        
        assert isinstance(name, basestring) and len(name) > 0
        assert isinstance(dataDir, basestring) and len(dataDir) > 0
        
        self.id = id
        self.name = name
        self.dataDir = dataDir
        self.roleTemplates = roleTemplates
        self.deployments = list(deployments)
        
    def createDeployment(self, cloud, roleCounts, awsCred=None, problemSize=None):
        
        assert isinstance(cloud, Cloud)
        assert len(roleCounts) <= len(self.roleTemplates)
        
        id = ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(6))
        dataDir = os.path.join(self.dataDir, id)
        deployment = Deployment(id, dataDir, cloud=cloud, deploymentTemplate=self, awsCred=awsCred, problemSize=problemSize)
        
        roles = []
        
        for roleTemp, (image, instanceType, count) in roleCounts.iteritems():
            assert roleTemp in self.roleTemplates
            if count > 0:
                roles.append(roleTemp.createRole(deployment, cloud, image, instanceType, count))
        
        deployment.roles = roles
        
        self.deployments.append(deployment)
        
        return deployment
        
        
class RoleTemplate(object):
    
    def __init__(self, id, name, image, roles=(), 
                 startActions=(), uploadActions=(),
                 finishedChecks=(), 
                 dataCollectors=(), launchCred=None, contextCred=None):
        self.id = id
        self.name = name
        self.image = image
        self.roles = list(roles)
        self.startActions= list(startActions)
        self.uploadActions= list(uploadActions)
        self.finishedChecks= list(finishedChecks)
        self.dataCollectors= list(dataCollectors)
        self.launchCred = launchCred
        self.contextCred = contextCred
        
    def createRole(self, deployment, cloud, image, instanceType, count):
        
        role = Role(None, image=image, count=count, instanceType=instanceType, 
                    template=self, deployment=deployment)
        
        '''
        Copy all actions from template. We want a copy of the action that may be modified without
        changing the template's version.
        '''
        role.startActions = [a.copy() for a in self.startActions]
        for a in role.startActions:
            a.id = None
            
        role.uploadActions = [a.copy() for a in self.uploadActions]
        for a in role.uploadActions:
            a.id = None
            
        role.finishedChecks = [a.copy() for a in self.finishedChecks]
        for a in role.finishedChecks:
            a.id = None
            
        role.dataCollectors = [a.copy() for a in self.dataCollectors]
        for a in role.dataCollectors:
            a.id = None
            
        return role
        
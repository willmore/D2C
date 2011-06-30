
class DeploymentTemplate(object):
    
    def __init__(self, id, dataDir, roleTemplates, deployments=()):
        self.id = id
        self.dataDir = dataDir,
        self.roleTemplates = roleTemplates
        self.deployments = list(deployments)
        
class RoleTemplate(object):
    
    def __init__(self, id, roles=(), 
                 startActions=(), uploadActions=(),
                 stopActions=(), finishedChecks=(), 
                 dataCollectors=(), launchCred=None, contextCred=None):
        self.id = id
        self.roles = list(roles)
        self.startActions= list(startActions)
        self.uploadActions= list(uploadActions)
        self.stopActions= list(stopActions)
        self.finishedChecks= list(finishedChecks)
        self.dataCollectors= list(dataCollectors)
        self.launchCred = launchCred
        self.contextCred = contextCred
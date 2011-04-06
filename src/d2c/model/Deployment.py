'''
Created on Mar 14, 2011

@author: willmore
'''

class Instance:
    '''
    Minimum storage of locally stored instance information. The rest of the instance attributes 
    should be fetched dynamically via boto.
    '''
    
    def __init__(self, id):
        self.id = id
        
    def __str__(self):
        return "{id: %s}" % self.id

class Role:
    
    def __init__(self, deploymentId, name, ami, count, instances=None):
        
        self.deploymentId = deploymentId
        self.name = name
        self.ami = ami
        
        assert count > 0
        self.count = count
        
        self.instances = instances if instances is not None else ()
        
    def __str__(self):
        return "{name:%s, ami: %s, instances: %s}" % (self.name, self.ami, str(self.instances))


class DeploymentState:
    NOT_RUN = 'NOT_RUN'
    RUNNING = 'RUNNING'
    FINALIZING = 'FINALIZING'
    COMPLETED = 'COMPLETED'
    
class StartAction:
    
    def __init__(self, role, script):
        self.role = role
        self.script = script
        
class DataCollection:
    
    def __init__(self, role, directory):
        self.role = role
        self.directory = directory

class Deployment:
    """
    Represents an instance of a Deployment.
    A deployment consists of one or more reservations, 
        which may be in various states (requested, running, terminated, etc.)
    """   
    
    def __init__(self, id, roles=[], startActions=(), dataCollections=(), 
                 reservations=(), state=DeploymentState.NOT_RUN):
        self.id = id
        self.roles = list(roles)
        self.reservations = list(reservations)
        self.startActions = list(startActions)
        self.state = state
    
    def getState(self):
        pass
    
    def addRole(self, role):
        print "Calling add role to " + str(self.roles)
        self.roles.append(role)
        
    def getInstances(self):
        '''
        Return iterable all instances (if any) associated with this deployment
        '''
        out = []
        for l in map(lambda role: role.instances, self.roles):
            out.extend(l)
            
        return out
        
    def __str__(self):
        return "{id:%s, roles:%s}" % (self.id,str(self.roles))
'''
Created on Mar 14, 2011

@author: willmore
'''

class Role:
    
    def __init__(self, name, ami, count):
        self.name = name
        self.ami = ami
        
        assert count > 0
        self.count = count
        
    def __str__(self):
        return "{name:%s, ami: %s}" % (self.name, self.ami)


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
        
    def __str__(self):
        return "{id:%s, roles:%s}" % (self.id,str(self.roles))
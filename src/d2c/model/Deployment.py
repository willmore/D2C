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
    Represents an instance of a DeploymentDescription.
    A deployment consists of one or more reservations, 
    which may be in various states (requested, running, terminated, etc.)
    """
    
    NEW_ID = -1
    
    def __init__(self, id, roles, startActions, dataCollections, reservations=(), state=DeploymentState.NOT_RUN):
        self.id = id
        self.roles = roles
        self.reservations = reservations
        self.startActions = startActions
        self.state = state
    
    def getState(self):
        pass
    
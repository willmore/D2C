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
    PENDING = 'PENDING'
    RUNNING = 'RUNNING'
    FINALIZING = 'FINALIZING'
    COMPLETED = 'COMPLETED'
    


class Deployment:
    """
    Represents an instance of a DeploymentDescription.
    A deployment consists of one or more reservations, 
    which may be in various states (requested, running, terminated, etc.)
    """
    
    def __init__(self, id, reservations, deploymentDesc):
        self.deploymentDesc = deploymentDesc
        self.reservations = reservations
    
    def getState(self):
        pass
    
'''
Created on Feb 10, 2011

@author: willmore
'''



        
class DeploymentDescription:
    """
    Describes how a group of instances should be provisioned.
    """
    
    
    def __init__(self, name, roles=[]):
                
        self.name = name
        self.roles = roles
        
    def addRole(self, role):
        self.roles.append(role)
        
        
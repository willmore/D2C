'''
Created on Feb 10, 2011

@author: willmore
'''


class Role:
    
    def __init__(self, name, ami, count):
        self.name = name
        self.ami = ami
        
        assert count > 0
        self.count = count
        
class DeploymentDescription:
    """
    Describes how a group of instances should be provisioned.
    """
    
    
    def __init__(self, name, roles=[]):
                
        self.name = name
        self.roles = roles
        
    def addRole(self, role):
        self.roles.append(role)
        
        
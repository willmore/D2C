'''
Created on Feb 10, 2011

@author: willmore
'''


class Role:
    
    def __init__(self, name, ami):
        self.name = name
        self.ami = ami
        
class RoleCountMap:

    def __init__(self, vals):        
        self.__map = {}
        if vals is not None:
            self.add(vals)
    
    def add(self, vals):
        for (role, count) in vals.iteritems():
            self.addRole(role, count)
    
    def addRole(self, role, count):
        assert role.__class__ == Role
        assert count > 0
        
        self.__map[role] = count
        
    def iteritems(self):
        return self.__map.iteritems()
        
class DeploymentDescription:
    """
    Describes how a group of instances should be provisioned.
    """
    
    
    def __init__(self, name, roleCountMap):
        
        assert roleCountMap.__class__ == RoleCountMap
        
        self.name = name
        self.roleCountMap = roleCountMap
        
        
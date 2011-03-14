'''
Created on Mar 14, 2011

@author: willmore
'''

class DeploymentDescController:
    
    def __init__(self, view, dao):
        self.view = view
        self.dao = dao
        
        self.view.setDescs(dao.getDeploymentDescriptions())
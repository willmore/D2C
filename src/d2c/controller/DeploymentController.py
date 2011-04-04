'''
Created on Feb 15, 2011

@author: willmore
'''
import wx

class DeploymentController:
    
    def __init__(self, deploymentView, dao):
        
        self.dao = dao
        self.deploymentView = deploymentView
'''
Created on Feb 15, 2011

@author: willmore
'''
import wx

class DeploymentController:
    
    def __init__(self, deploymentView, dao):
        
        self.dao = dao
        self.deploymentView = deploymentView
        self.deploymentView.deployButton.Bind(wx.EVT_BUTTON, self.handleLaunch)
        
        self.deployment = deploymentView.deployment
        
        
    def handleLaunch(self, evt):
        print "Launching %s " % self.deployment
    
    def handleStart(self, evt):
        pass
        
    def handleKill(self, evt):
        pass
    
    
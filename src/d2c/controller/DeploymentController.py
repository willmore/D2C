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
        
        ret = wx.MessageBox('Are you sure you want to start? AWS charges will start.', 'Question', wx.YES_NO)
        if wx.YES == ret:
            print "Launching %s " % self.deployment
    
    def handleStart(self, evt):
        pass
        
    def handleKill(self, evt):
        pass
    
    
'''
Created on Feb 15, 2011

@author: willmore
'''
import wx
from threading import Thread

class DeploymentThread(Thread):
        
        def __init__(self, deployment):
            Thread.__init__(self)
            self.deployment = deployment
            
        def run(self):
            self.deployment.run()
              
        def pause(self):
            self.deployment.pause()
            
class PersistenceListener:
    
    def __init__(self, dao):
        self.dao = dao

    def notify(self, event):
        print "Deployment changed"
        
class ViewListener:
    
    def __init__(self, deploymentView):
        self.deploymentView = deploymentView
        
    def notify(self, event):
        self.deploymentView.update()
        
    
class DeploymentController:
    
    def __init__(self, deploymentView, dao):
        
        self.dao = dao
        self.deploymentView = deploymentView
        self.deploymentView.deployButton.Bind(wx.EVT_BUTTON, self.handleLaunch)
        
        self.deployment = deploymentView.deployment
        
    

    def handleLaunch(self, evt):
        
        ret = wx.MessageBox('Are you sure you want to start? AWS charges will start.', 'Question', wx.YES_NO)
        if wx.YES == ret:
            self.deployment.addAnyStateChangeListener(PersistenceListener(self.dao))
            self.deployment.addAnyStateChangeListener(ViewListener(self.deploymentView))
            self.deploymentThread = DeploymentThread(self.deployment)
            self.deploymentThread.start()
    
    def handleStart(self, evt):
        pass
        
    def handleKill(self, evt):
        pass
    
    
    
    
'''
Created on Feb 15, 2011

@author: willmore
'''
import wx
from threading import Thread
from wx.lib.pubsub import Publisher


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
        
        ret = wx.MessageBox('Are you sure you want to start? \
                            AWS charges will start at the rate of $%.2f per hour' 
                            % self.deployment.costPerHour(), 'Question', wx.YES_NO)
        
        if wx.YES == ret:
            
            self.deploymentView.deployButton.Hide()
            
            
            self.deploymentView.showLogPanel()
            
            self.__createLogger()
            
            self.deployment.addAnyStateChangeListener(PersistenceListener(self.dao))
            self.deployment.addAnyStateChangeListener(ViewListener(self.deploymentView))
            self.deploymentThread = DeploymentThread(self.deployment)
            self.deploymentThread.start()
    
    def __createLogger(self):
        channelId = self.deployment.id
        
        logger = self.__DeploymentLogger(channelId, self.deploymentView)
        
        Publisher.subscribe(logger.receiveMsg, 
                            channelId)
                
        self.deployment.logger = logger
        
        return logger
    
    class __DeploymentLogger:
        
        def __init__(self, channel, logPanel):
            self.channel = channel
            self.logPanel = logPanel
            
        def write(self, msg):
            print "Send message to channel %s " % self.channel
            wx.CallAfter(Publisher.sendMessage, self.channel, msg)
           
        def receiveMsg(self, msg):
            print "Receive msg"
            self.logPanel.appendLogPanelText(msg.data) 
    
    def handleStart(self, evt):
        pass
        
    def handleKill(self, evt):
        pass
    
    
    
    

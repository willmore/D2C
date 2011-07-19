import wx
from threading import Thread
from wx.lib.pubsub import Publisher
import sys

class DeploymentThread(Thread):
        
    def __init__(self, deployment):
        Thread.__init__(self)
        self.deployment = deployment
            
    def run(self):
        try:
            self.deployment.run()
        except Exception as x:
            wx.CallAfter(Publisher.sendMessage, "DEPLOYMENT EXCEPTION", x)
            #TODO stop deployment
              
    def pause(self):
        self.deployment.pause()
            
class PersistenceListener:
    
    def __init__(self, dao, deployment):
        self.dao = dao
        self.deployment = deployment

    def notify(self, _):
        self.dao.save(self.deployment)
        
class ViewListener:
    
    def __init__(self, deploymentView):
        self.deploymentView = deploymentView
        
    def notify(self, _):
        wx.CallAfter(self.deploymentView.update)     
    
    
class DeploymentController:
    
    def __init__(self, deploymentView, dao):
        
        self.dao = dao
        self.deploymentView = deploymentView
        self.deploymentView.deployButton.Bind(wx.EVT_BUTTON, self.handleLaunch)
        self.deploymentView.cancelButton.Bind(wx.EVT_BUTTON, self.handleCancel)
        
        self.deployment = deploymentView.deployment
        
        Publisher.subscribe(self.handleException, 
                            "DEPLOYMENT EXCEPTION")
    
    def handleException(self, msg):
        
        wx.MessageBox("Unexpected error: %s" % str(msg.data), 'Error!', wx.ICON_ERROR)

    def handleCancel(self, _):
        
        ret = wx.MessageBox('Are you sure you want to cancel?',
                            'Question', wx.YES_NO)
        
        if wx.YES == ret:
               
            self.deploymentView.showLogPanel()
            
            self.deployment.stop()
            self.deploymentView.cancelButton.Hide()
            

    def handleLaunch(self, _):
        
        ret = wx.MessageBox('Are you sure you want to start? \
                            AWS charges will start at the rate of $%.2f per hour' 
                            % self.deployment.costPerHour(), 'Question', wx.YES_NO)
        
        if wx.YES == ret:
            
            self.deploymentView.deployButton.Hide()
            
            self.deploymentView.showLogPanel()
            
            self.__createLogger()
            
            self.deployment.addAnyStateChangeListener(PersistenceListener(self.dao, self.deployment))
            self.deployment.addAnyStateChangeListener(ViewListener(self.deploymentView))
            self.deploymentThread = DeploymentThread(self.deployment)
            self.deploymentThread.start()
            self.deploymentView.cancelButton.Show()
            self.deploymentView.Layout()
    
    def __createLogger(self):
        channelId = self.deployment.id
        
        logger = self.__DeploymentLogger(channelId, self.deploymentView)
        
        Publisher.subscribe(logger.receiveMsg, 
                            channelId)
                
        self.deployment.setLogger(logger)
        
        return logger
    
    class __DeploymentLogger:
        
        def __init__(self, channel, logPanel):
            self.channel = channel
            self.logPanel = logPanel
            
        def write(self, msg):
            wx.CallAfter(Publisher.sendMessage, self.channel, msg)
           
        def receiveMsg(self, msg):
            self.logPanel.appendLogPanelText(msg.data)
            
class DeploymentTemplateController:
    
    def __init__(self, deploymentView, dao):
        
        self.dao = dao
        self.deploymentView = deploymentView
        
        self.deployment = deploymentView.deployment 

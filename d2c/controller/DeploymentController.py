import wx
from threading import Thread
from wx.lib.pubsub import Publisher
from d2c.gui.RolePanel import RoleDialog
from .RolePanelController import RolePanelController
import sys
import traceback

class DeploymentThread(Thread):
        
    def __init__(self, deployment):
        Thread.__init__(self)
        self.deployment = deployment
            
    def run(self):
        try:
            self.deployment.run()
        except Exception as x:
            exc_type, exc_value, exc_traceback = sys.exc_info()
            trace = "".join(traceback.format_exception(exc_type, exc_value, exc_traceback))
            wx.CallAfter(Publisher.sendMessage, "DEPLOYMENT EXCEPTION", (x,trace))
              
    def pause(self):
        self.deployment.pause()

            
class PersistenceListener:
    '''
    Saves in DB when the deployment state changes.
    '''
    
    def __init__(self, dao, deployment):
        self.dao = dao
        self.deployment = deployment

    def notify(self, _):
        self.dao.save(self.deployment)
     
        
class ViewListener:
    '''
    Notifies the View of deployment state changes.
    '''
    
    def __init__(self, deploymentView):
        self.view = deploymentView
        
    def notify(self, _):
        wx.CallAfter(self.view.update)     
    
    
class DeploymentController:
    
    def __init__(self, deploymentView, dao):
        
        self.dao = dao
        self.view = deploymentView
        self.view.deployButton.Bind(wx.EVT_BUTTON, self.handleLaunch)
        self.view.cancelButton.Bind(wx.EVT_BUTTON, self.handleCancel)
        
        self.deployment = deploymentView.deployment
        
        Publisher.subscribe(self.handleException, 
                            "DEPLOYMENT EXCEPTION")
        
        self.view.roles.Bind(wx.EVT_LIST_ITEM_RIGHT_CLICK, self.onRolesRightClick)
    
    def onRolesRightClick(self, event):
        item, flags = self.view.roles.HitTest(event.GetPosition())
        if flags == wx.NOT_FOUND:
            event.Skip()
            return
        self.view.roles.Select(item)
        
        self.view.roles.PopupMenu(RolePopupMenu(self.view.roles.getSelectedItems()[0], self.dao),
                                  event.GetPosition())
    
    def handleException(self, msg):     
        wx.MessageBox("Unexpected error: %s\nTrace%s" % (str(msg.data[0]), msg.data[1]), 'Error!', wx.ICON_ERROR)

    def handleCancel(self, _):  
        ret = wx.MessageBox('Are you sure you want to cancel?',
                            'Question', wx.YES_NO)
        
        if wx.YES == ret:
               
            self.view.showLogPanel()
            
            self.deployment.stop()
            self.view.cancelButton.Hide()
            
    def handleLaunch(self, _):       
        ret = wx.MessageBox('Are you sure you want to start? \
                            AWS charges will start at the rate of $%.2f per hour' 
                            % self.deployment.costPerHour(), 'Question', wx.YES_NO)
        
        if wx.YES == ret:
            
            self.view.deployButton.Hide()
            
            self.view.showLogPanel()
            
            self.__createLogger()
            
            self.deployment.addAnyStateChangeListener(PersistenceListener(self.dao, self.deployment))
            self.deployment.addAnyStateChangeListener(ViewListener(self.view))
            self.deploymentThread = DeploymentThread(self.deployment)
            self.deploymentThread.start()
            self.view.cancelButton.Show()
            self.view.Layout()
    
    def __createLogger(self):
        channelId = self.deployment.id
        
        logger = self.__DeploymentLogger(channelId, self.view)
        
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

class RolePopupMenu(wx.Menu):
    def __init__(self, role, dao):
        wx.Menu.__init__(self)
        
        self.dao = dao
        self.role = role
        
        item = wx.MenuItem(self, wx.NewId(), "View / Edit Role")
        self.AppendItem(item)
        self.Bind(wx.EVT_MENU, self.onEditRole, item)

    def onEditRole(self, _):
        print "Item One selected in the %s" % self.role.template.name
        
        roleDialog = RoleDialog(None, -1, size=(400,400))
        
        RolePanelController(roleDialog, self.role, self.dao)
        
        roleDialog.ShowModal()
        roleDialog.Destroy()
      
class DeploymentTemplateController:
    
    def __init__(self, deploymentView, dao):
        
        self.dao = dao
        self.view = deploymentView
        
        self.deployment = deploymentView.deployment 

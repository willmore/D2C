
import wx
import datetime     
from .RoleList import RoleList
from d2c.model.Deployment import DeploymentState
from d2c.gui.ItemList import ItemList, ColumnMapper


def formatTime(t):
    return datetime.datetime.fromtimestamp(t).isoformat()

class DeploymentPanel(wx.Panel):    
    
    '''
    Renders Deployment information
    '''
    
    def __init__(self, deployment, *args, **kwargs):
        wx.Panel.__init__(self, *args, **kwargs)
        
        self.SetSizer(wx.BoxSizer(wx.VERTICAL))
        
        self.deployment = deployment
        
        label = wx.StaticText(self, -1, deployment.deploymentTemplate.name + ":" + str(deployment.id))
        label.SetFont(wx.Font(20, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.GetSizer().Add(label, 0, wx.BOTTOM, 10)
        
        self.tabContainer = wx.Notebook(self, -1, style=wx.NB_TOP)
        self.GetSizer().Add(self.tabContainer, 1, wx.ALL | wx.EXPAND, 2)
        
        self.overviewTab = OverviewTab(self.deployment, self.tabContainer, -1)
        self.tabContainer.AddPage(self.overviewTab, "Overview")
        
        self.eventTab = EventTab(self.deployment, self.tabContainer, -1)
        self.tabContainer.AddPage(self.eventTab, "Log / Events")
        
        self.monitorTab = EventTab(self.deployment, self.tabContainer, -1)
        self.tabContainer.AddPage(self.monitorTab, "Monitoring")
        
    def showLogPanel(self):
        self.eventTab.showLogPanel()
        
    def appendLogPanelText(self, text):
        self.eventTab.appendLogPanelText(text)
    
    def update(self):
        '''
        Update the panel to reflect the current Deployment
        '''
        self.overviewTab.update()
        self.eventTab.update()
        
class MonitorTab(wx.Panel):
    '''
    Displays graphs of system resource usage.
    This will only contain data after completion of a deployment.
    '''
    
    def __init__(self, *args, **kwargs):
         
        wx.Panel.__init__(self, *args, **kwargs) 
  
class EventTab(wx.Panel):
    '''
    Displays Deployment events and log.
    '''   
        
    def __init__(self, deployment, *args, **kwargs):
        
        wx.Panel.__init__(self, *args, **kwargs)    
        
        self.deployment = deployment
        self.SetSizer(wx.BoxSizer(wx.VERTICAL))
        
        self.eventLabel = wx.StaticText(self, -1, label="Events")
        self.eventLabel.SetFont(wx.Font(wx.DEFAULT, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.GetSizer().Add(self.eventLabel, 0, wx.ALL, 2)
        
        self.eventList = ItemList(self, -1, 
                                  mappers=[ColumnMapper('Event', lambda e: e.state, defaultWidth=wx.LIST_AUTOSIZE),
                                           ColumnMapper('Time', lambda e: formatTime(e.time), defaultWidth=wx.LIST_AUTOSIZE)],
                                  style=wx.LC_REPORT)
        self.GetSizer().Add(self.eventList, 0, wx.ALL | wx.EXPAND, 2)
        
        self.logPanel = wx.TextCtrl(self, -1, size=(100,100), style=wx.TE_MULTILINE )
        self.logLabel = wx.StaticText(self, -1, label="Log")
        self.logLabel.SetFont(wx.Font(wx.DEFAULT, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.GetSizer().Add(self.logLabel, 0, wx.ALL, 2)
        self.logLabel.Hide()
        self.logPanel.Hide() # logPanel will be shown later on demand
        self.GetSizer().Add(self.logPanel, 1, wx.BOTTOM | wx.EXPAND, 5)
        
    def update(self):
        self.eventList.setItems(self.deployment.stateEvents)
    
    def appendLogPanelText(self, text):
        self.logPanel.AppendText(str(text) + "\n")
            
    def showLogPanel(self):
        self.logPanel.Show()
        self.logLabel.Show()
        self.Layout()
        
class OverviewTab(wx.Panel):
    '''
    Displays basic information about the Deployment.
    '''
    
    def __init__(self, deployment, *args, **kwargs):
        
        wx.Panel.__init__(self, *args, **kwargs)
        
        self.deployment = deployment
        self.SetSizer(wx.BoxSizer(wx.VERTICAL))
                
        sizer = wx.BoxSizer(wx.HORIZONTAL)
        self.GetSizer().Add(sizer, 0, wx.BOTTOM, 5)
        label = wx.StaticText(self, -1, 'Cloud')
        label.SetFont(wx.Font(wx.DEFAULT, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        sizer.Add(label, 0, wx.RIGHT, 5)
        self.cloudField = wx.StaticText(self, -1, deployment.cloud.name)
        sizer.Add(self.cloudField, 0, wx.ALIGN_CENTER)
        
        self.sizeSizer = wx.BoxSizer(wx.HORIZONTAL)
        self.GetSizer().Add(self.sizeSizer, 0, wx.BOTTOM, 5)
        label = wx.StaticText(self, -1, 'Problem Size')
        label.SetFont(wx.Font(wx.DEFAULT, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.sizeSizer.Add(label, 0, wx.RIGHT, 5)
        self.sizeField = wx.StaticText(self, -1, str(deployment.problemSize))
        self.sizeSizer.Add(self.sizeField, 0, wx.ALIGN_CENTER)
        
        self.statusSizer = wx.BoxSizer(wx.HORIZONTAL)
        self.GetSizer().Add(self.statusSizer, 0, wx.BOTTOM, 5)
        label = wx.StaticText(self, -1, 'Status')
        label.SetFont(wx.Font(wx.DEFAULT, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.statusSizer.Add(label, 0, wx.RIGHT, 5)
        self.statusField = wx.StaticText(self, -1, deployment.state)
        self.statusSizer.Add(self.statusField, 0, wx.ALIGN_CENTER)
                  
        label = wx.StaticText(self, -1, 'Roles')
        label.SetFont(wx.Font(wx.DEFAULT, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.GetSizer().Add(label, 0, wx.BOTTOM, 5)
        self.roles = RoleList(self, -1, items=deployment.roles)
        self.GetSizer().Add(self.roles, 0, wx.BOTTOM | wx.EXPAND, 5)
        
        self.dataPanel = wx.Panel(self,-1)
        self.GetSizer().Add(self.dataPanel)
        self.dataPanel.SetSizer(wx.BoxSizer(wx.VERTICAL))
        self.dataPanel.Hide()
        
        label = wx.StaticText(self.dataPanel, -1, 'Role Data')
        label.SetFont(wx.Font(wx.DEFAULT, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.dataPanel.GetSizer().Add(label, 0, wx.BOTTOM, 5)
        
        for role in self.deployment.roles:
            hbox = wx.BoxSizer(wx.HORIZONTAL)
            label = wx.StaticText(self.dataPanel, -1, role.getName())
            hbox.Add(label, 0, wx.EXPAND | wx.ALL, 2)
            hbox.Add(wx.StaticText(self.dataPanel, -1, role.getDataDirectory()), 1, wx.EXPAND | wx.ALL, 2)
            self.dataPanel.GetSizer().Add(hbox, 0, wx.EXPAND | wx.ALL, 2)
        
        self.deployButton = wx.Button(self, wx.ID_ANY, 'Deploy', size=(110, -1))
        self.GetSizer().Add(self.deployButton, 0, wx.ALL, 2)
        
        self.cancelButton = wx.Button(self, wx.ID_ANY, 'Cancel', size=(110, -1))
        self.GetSizer().Add(self.cancelButton, 0, wx.ALL, 2)
        
        self.cancelButton.Hide()
        self.update()
        
        
    
    def update(self):
        '''
        Update the panel to reflect the current Deployment
        '''
        self.statusField.SetLabel(self.deployment.state)
        
        if self.deployment.state != DeploymentState.NOT_RUN:
            self.deployButton.Hide()
        
        if self.deployment.state == DeploymentState.COMPLETED:
            self.cancelButton.Hide()
            self.dataPanel.Show()
    
            
        


import wx
from .RoleList import RoleList
from d2c.model.Deployment import DeploymentState
from d2c.gui.ItemList import ItemList, ColumnMapper
from d2c.graph.Grapher import Grapher
import datetime


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
        self.GetSizer().Add(label, 0, wx.BOTTOM | wx.TOP, 10)
        
        self.tabContainer = wx.Notebook(self, -1, style=wx.NB_TOP)
        self.GetSizer().Add(self.tabContainer, 1, wx.ALL | wx.EXPAND, 5)
         
        self.overviewTab = OverviewTab(self.deployment, self.tabContainer, -1)
        self.tabContainer.AddPage(self.overviewTab, "Overview")
        
        self.eventTab = EventTab(self.deployment, self.tabContainer, -1)
        #self.eventTab.update();
        self.tabContainer.AddPage(self.eventTab, "Log / Events")
        
        self.monitorTab = MonitorTab(self.deployment, self.tabContainer, -1)
        self.tabContainer.AddPage(self.monitorTab, "Monitoring")
        
        hbox = wx.BoxSizer(wx.HORIZONTAL)
        self.GetSizer().Add(hbox)
        self.cloneButton = wx.Button(self, -1 , "Clone")
        hbox.Add(self.cloneButton, 0, wx.ALL, 5)
        
        self.deleteButton = wx.Button(self, -1 , "Delete")
        hbox.Add(self.deleteButton, 0, wx.ALL, 5)
        
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
    
    def __init__(self, deployment, *args, **kwargs):
        wx.Panel.__init__(self, *args, **kwargs)
        
        self.deployment = deployment
         
        self.sizer = wx.BoxSizer(wx.VERTICAL)
        self.SetSizer(self.sizer)
         
        self.sw = wx.ScrolledWindow(self, style=wx.VSCROLL)
        self.sizer.Add(self.sw, 1, wx.ALL | wx.EXPAND, 5)
        
        self.sw.SetScrollbars(1,1,1,1)
        self.sw.SetMinSize((-1, 50))
        
        self.sw.SetScrollbars(20,20,55,40)
        self.sw.SetSizer(wx.BoxSizer(wx.VERTICAL))
        
    def graph(self):
        #grapher = Grapher({"vm":"/home/willmore/.d2c/deployments/22.py/QCW7N1/31/i-4B450863/opt/collectd/var/lib/collectd/dirac.lan/",
        #           "vm2":"/home/willmore/.d2c/deployments/22.py/QCW7N1/32/i-468F08BD/opt/collectd/var/lib/collectd/dirac.lan/"
        #           }, "/tmp")


        collectdPaths = []
        for role in self.deployment.roles:
            collectdPaths.extend(role.getIntsanceCollectdDirs())
        
        if len(collectdPaths) == 0:
            print "No Data"
            return
        
        print "Graph data"
        
        graphMap = {}
        for n,path in enumerate(collectdPaths):
            print "Adding path: " + path
            graphMap["vm"+str(n)] = str(path)

        print "Graphing " + str(graphMap)
        grapher = Grapher(graphMap, "/tmp")

        print self.deployment.getRoleStartTime()
        startString = datetime.datetime.fromtimestamp(self.deployment.getRoleStartTime()).strftime('%Y%m%d %H:%M')
        endString = datetime.datetime.fromtimestamp(self.deployment.getRoleEndTime()).strftime('%Y%m%d %H:%M')
        
        cpuImg = grapher.generateCPUGraphsAverage(startString, endString)
        memImg = grapher.generateMemoryGraph(startString, endString)
        netImg = grapher.generateNetworkGraph(startString, endString)
        
        bmp = wx.Image(cpuImg,wx.BITMAP_TYPE_PNG).ConvertToBitmap()
       
        hsizer = wx.BoxSizer(wx.HORIZONTAL)
        hsizer.Add(wx.StaticBitmap(self.sw, -1, bmp))
        self.sw.GetSizer().Add(hsizer)
       
        bmp = wx.Image(memImg,wx.BITMAP_TYPE_PNG).ConvertToBitmap()
        hsizer = wx.BoxSizer(wx.HORIZONTAL)
        hsizer.Add(wx.StaticBitmap(self.sw, -1, bmp))
        self.sw.GetSizer().Add(hsizer)
        
        bmp = wx.Image(netImg,wx.BITMAP_TYPE_PNG).ConvertToBitmap()
        hsizer = wx.BoxSizer(wx.HORIZONTAL)
        hsizer.Add(wx.StaticBitmap(self.sw, -1, bmp))
        self.sw.GetSizer().Add(hsizer)
        
  
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
        self.GetSizer().Add(sizer, 0, wx.BOTTOM | wx.TOP, 5)
        label = wx.StaticText(self, -1, 'Cloud')
        label.SetFont(wx.Font(wx.DEFAULT, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        sizer.Add(label, 0, wx.RIGHT| wx.LEFT, 5)
        self.cloudField = wx.StaticText(self, -1, deployment.cloud.name)
        sizer.Add(self.cloudField, 0, wx.ALIGN_CENTER)
        
        self.sizeSizer = wx.BoxSizer(wx.HORIZONTAL)
        self.GetSizer().Add(self.sizeSizer, 0, wx.BOTTOM, 5)
        label = wx.StaticText(self, -1, 'Problem Size')
        label.SetFont(wx.Font(wx.DEFAULT, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.sizeSizer.Add(label, 0, wx.RIGHT| wx.LEFT, 5)
        self.sizeField = wx.StaticText(self, -1, str(deployment.problemSize))
        self.sizeSizer.Add(self.sizeField, 0, wx.ALIGN_CENTER)
        
        self.statusSizer = wx.BoxSizer(wx.HORIZONTAL)
        self.GetSizer().Add(self.statusSizer, 0, wx.BOTTOM, 5)
        label = wx.StaticText(self, -1, 'Status')
        label.SetFont(wx.Font(wx.DEFAULT, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.statusSizer.Add(label, 0, wx.RIGHT | wx.LEFT, 5)
        self.statusField = wx.StaticText(self, -1, deployment.state)
        self.statusSizer.Add(self.statusField, 0, wx.ALIGN_CENTER)
                  
        label = wx.StaticText(self, -1, 'Roles')
        label.SetFont(wx.Font(wx.DEFAULT, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.GetSizer().Add(label, 0, wx.BOTTOM | wx.LEFT, 5)
        self.roles = RoleList(self, -1, items=deployment.roles)
        self.GetSizer().Add(self.roles, 0, wx.ALL | wx.EXPAND, 5)
        
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
    
            
        

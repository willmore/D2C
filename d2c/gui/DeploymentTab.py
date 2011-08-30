import wx

from .ContainerPanel import ContainerPanel
from .RoleList import RoleTemplateList
from .DeploymentPanel import DeploymentPanel
from d2c.controller.DeploymentController import DeploymentController
from d2c.controller.DeploymentCreatorController import DeploymentCreatorController

from .DeploymentCreator import DeploymentCreator

from d2c.model.CompModel import GustafsonCompModel2

from pylab import *
from numpy import *
import numpy
from mpl_toolkits.mplot3d import Axes3D

from matplotlib.backends.backend_wxagg import FigureCanvasWxAgg as FigureCanvas

from matplotlib.backends.backend_wx import NavigationToolbar2Wx

#from matplotlib.figure import Figure

class DeploymentTab(wx.Panel):

    '''
    Allows navigation of all DeploymentTemplates and Deployments.
    The main panel is split between and left panel with tree-navigation 
    and the right panel displaying the selected DeploymentTemplate or DeploymentPanel.
    '''
    
    def __init__(self, dao, *args, **kwargs):

        wx.Panel.__init__(self, *args, **kwargs)
        
        self.dao = dao
        
        self.splitter = wx.SplitterWindow(self, -1)
        self.splitter.SetMinimumPaneSize(200)
        vbox = wx.BoxSizer(wx.VERTICAL)

        self.tree = wx.TreeCtrl(self.splitter, -1, wx.DefaultPosition, 
                                (-1,-1), wx.TR_HIDE_ROOT|wx.TR_HAS_BUTTONS)
        self.treeRoot = self.tree.AddRoot('root')
        vbox.Add(self.splitter, 1, wx.EXPAND)
        
        self.displayPanel = ContainerPanel(self.splitter, -1)
   
        self.splitter.SplitVertically(self.tree, self.displayPanel)
        self.SetSizer(vbox)
        self.Layout()   
        
    def addDeploymentTemplatePanel(self, deploymentPanel):
        
        node = self.tree.AppendItem(self.treeRoot, deploymentPanel.deployment.name)
        self.displayPanel.addPanel(deploymentPanel.deployment.name, deploymentPanel)
        
        for d in deploymentPanel.deployment.deployments:
            self.addDeployment(d, node)
            
    def addDeployment(self, deployment, parentNode=None):
    
    
        if parentNode is None:
            #Find parent node based on deployment name
            parentNode = self._getTreeItemIdByName(deployment.deploymentTemplate.name)
    
        label = deployment.deploymentTemplate.name  + ":" + str(deployment.id)
        self.tree.AppendItem(parentNode, label)
        p = DeploymentPanel(deployment, self.displayPanel, -1)
        self.displayPanel.addPanel(label, p)
        DeploymentController(p, self.dao)
        
    def _getTreeItemIdByName(self, name):
        '''
        Only iterates through root children
        '''
        item, cookie = self.tree.GetFirstChild(self.treeRoot)
        while item:
            
            if name == self.tree.GetItemText(item):
                return item
            
            item, cookie = self.tree.GetNextChild(self.treeRoot, cookie)
        
        return None
        
class DeploymentTemplatePanel(wx.Panel):    
    
    def __init__(self, deployment, dao, *args, **kwargs):
        
        wx.Panel.__init__(self, *args, **kwargs)
        self.dao = dao
        
        self.SetSizer(wx.BoxSizer(wx.VERTICAL))
        
        self.deployment = deployment
        
        label = wx.StaticText(self, -1, deployment.name)
        label.SetFont(wx.Font(20, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.GetSizer().Add(label, 0, wx.BOTTOM, 10)
          
          
        #self.GetSizer().Add(self.roles, 0, wx.BOTTOM | wx.EXPAND, 5)
        
        self.deployButton = wx.Button(self, wx.ID_ANY, 'Create Deployment')
        
        self.deployButton.Bind(wx.EVT_BUTTON, self.showDeploymentCreation)
        self.GetSizer().Add(self.deployButton, 0, wx.ALL, 2)
        
        self.tabContainer = wx.Notebook(self, -1, style=wx.NB_TOP)
        hbox = wx.BoxSizer(wx.HORIZONTAL)
        hbox.Add(self.tabContainer, 1, wx.ALL | wx.EXPAND, 2)
        self.GetSizer().Add(hbox, 1, wx.ALL | wx.EXPAND, 2)
        
        self.overviewTab = wx.Panel(self.tabContainer, -1)
        self.overviewTab.SetSizer(wx.BoxSizer(wx.VERTICAL))
        label = wx.StaticText(self.overviewTab, -1, 'Roles')
        label.SetFont(wx.Font(wx.DEFAULT, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.overviewTab.GetSizer().Add(label, 0, wx.BOTTOM, 5)
        self.overviewTab.roles = RoleTemplateList(self.overviewTab, -1, items=deployment.roleTemplates)        
        self.overviewTab.GetSizer().Add(self.overviewTab.roles, 1, wx.BOTTOM | wx.EXPAND, 5)
        
        self.tabContainer.AddPage(self.overviewTab, "Overview")
        
        self.experimentTab = CanvasPanel(deployment, self.tabContainer)
        self.tabContainer.AddPage(self.experimentTab, "Experiments")
        
        self.experimentTab = CostGraphPanel(deployment, self.dao.getClouds(), self.tabContainer)
        self.tabContainer.AddPage(self.experimentTab, "Cost Prediction")
        
        #self.overviewTab.Bind(wx.EVT_NOTEBOOK_PAGE_CHANGED)
        
    def showDeploymentCreation(self, _):
        c = DeploymentCreator(self, self.deployment, style=wx.DEFAULT_DIALOG_STYLE | wx.RESIZE_BORDER)
        DeploymentCreatorController(c, self.dao)
        c.ShowModal()
        
class CanvasPanel(wx.Panel):

    def __init__(self, deploymentTemplate, parent):
        
        self.deploymentTemplate = deploymentTemplate
        
        wx.Panel.__init__(self,parent,-1,
                         size=(550,350))
        self.sizer = wx.BoxSizer(wx.VERTICAL)
        self.chartButton = wx.Button(self, -1, 'Chart Experiments')
        self.sizer.Add(self.chartButton, 0)
        self.SetSizer(self.sizer)
        
        self.chartButton.Bind(wx.EVT_BUTTON, self.handleShow)
        
    def handleShow(self, evt):
           
        #self.Bind(wx.EVT_, self.handleShow)        
        model = GustafsonCompModel2(self.deploymentTemplate, scaleFunction='linear')
        self.SetBackgroundColour(wx.NamedColor("WHITE"))

        probSize = arange(80,6000,100)
        numProcs = arange(1, 16, 1)
    
        probSize, numProcs = np.meshgrid(probSize, numProcs)
        time = model.modelFunc(probSize, 2, numProcs)
        self.figure = Figure()
        ax = Axes3D(self.figure)
        colortuple = ('y', 'b')
        colors = np.empty(probSize.shape, dtype=str)
        
        rstride = 1#(max([dp.machineCount for dp in points]) - min([dp.machineCount for dp in points])) / 20
        cstride = int(math.ceil(true_divide(max([d.problemSize for d in self.deploymentTemplate.deployments]) - 
                          min([d.problemSize for d in self.deploymentTemplate.deployments]), 45)))
        
        surf = ax.plot_wireframe(probSize, numProcs, time, rstride=rstride, cstride=cstride,
                 antialiased=False)
        
        #for p in points:
        #    ax.scatter(array([p.probSize]), array([p.machineCount]), array([p.time]), c='r', marker='o')
                
        #ax.set_zlim3d(0, max([p.time for p in points]))
        #ax.set_xlim3d(min([p.probSize for p in points]), maxProbSize)
        ax.w_zaxis.set_major_locator(LinearLocator(6))
        ax.w_zaxis.set_major_formatter(FormatStrFormatter('%.03f'))
        ax.w_yaxis.set_major_formatter(FormatStrFormatter('%d'))
        ax.w_xaxis.set_major_formatter(FormatStrFormatter('%d'))
        #self.figure = Figure()
        #self.axes = self.figure.add_subplot(111)
        #t = arange(0.0,3.0,0.01)
        #s = sin(2*pi*t)

        
        #########################
        self.canvas = FigureCanvas(self, -1, self.figure)

        
        self.sizer.Add(self.canvas, 1, wx.LEFT | wx.TOP | wx.GROW)
        
        self.Fit()

        self.add_toolbar()  # comment this out for no toolbar


    def add_toolbar(self):
        self.toolbar = NavigationToolbar2Wx(self.canvas)
        self.toolbar.Realize()
        if wx.Platform == '__WXMAC__':
            # Mac platform (OSX 10.3, MacPython) does not seem to cope with
            # having a toolbar in a sizer. This work-around gets the buttons
            # back, but at the expense of having the toolbar at the top
            self.SetToolBar(self.toolbar)
        else:
            # On Windows platform, default window size is incorrect, so set
            # toolbar width to figure width.
            tw, th = self.toolbar.GetSizeTuple()
            fw, fh = self.canvas.GetSizeTuple()
            # By adding toolbar in sizer, we are able to put it at the bottom
            # of the frame - so appearance is closer to GTK version.
            # As noted above, doesn't work for Mac.
            self.toolbar.SetSize(wx.Size(fw, th))
            self.sizer.Add(self.toolbar, 0, wx.LEFT | wx.EXPAND)
        # update the axes menu on the toolbar
        self.toolbar.update()


    def OnPaint(self, event):
        self.canvas.draw()
        
class CostGraphPanel(wx.Panel):


    def __init__(self, deploymentTemplate, clouds, parent):
        
        self.clouds = clouds
        
        self.deploymentTemplate = deploymentTemplate
        
        wx.Panel.__init__(self,parent,-1,
                         size=(550,350))
        self.sizer = wx.BoxSizer(wx.VERTICAL)
        self.chartButton = wx.Button(self, -1, 'Chart Experiments')
        self.sizer.Add(self.chartButton, 0)
        self.SetSizer(self.sizer)
        
        self.chartButton.Bind(wx.EVT_BUTTON, self.handleShow)
        
    def handleShow(self, evt):
           
        #self.Bind(wx.EVT_, self.handleShow)
        
        model = GustafsonCompModel2(self.deploymentTemplate, scaleFunction='linear')
        self.SetBackgroundColour(wx.NamedColor("WHITE"))
        
        self.figure = Figure()
        self.canvas = FigureCanvas(self, -1, self.figure)
        self.axes = self.figure.add_subplot(111)
        t = arange(80,6000,100)
        
        for cloud in self.clouds:
            s = model.costModel(cloud)(t)
            self.axes.plot(t,s)
        self.sizer.Add(self.canvas, 1, wx.LEFT | wx.TOP | wx.GROW)
        
        self.Fit()

        self.add_toolbar()  # comment this out for no toolbar


    def add_toolbar(self):
        self.toolbar = NavigationToolbar2Wx(self.canvas)
        self.toolbar.Realize()
        if wx.Platform == '__WXMAC__':
            # Mac platform (OSX 10.3, MacPython) does not seem to cope with
            # having a toolbar in a sizer. This work-around gets the buttons
            # back, but at the expense of having the toolbar at the top
            self.SetToolBar(self.toolbar)
        else:
            # On Windows platform, default window size is incorrect, so set
            # toolbar width to figure width.
            tw, th = self.toolbar.GetSizeTuple()
            fw, fh = self.canvas.GetSizeTuple()
            # By adding toolbar in sizer, we are able to put it at the bottom
            # of the frame - so appearance is closer to GTK version.
            # As noted above, doesn't work for Mac.
            self.toolbar.SetSize(wx.Size(fw, th))
            self.sizer.Add(self.toolbar, 0, wx.LEFT | wx.EXPAND)
        # update the axes menu on the toolbar
        self.toolbar.update()


    def OnPaint(self, event):
        self.canvas.draw()
        
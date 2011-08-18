
import wx
from .ItemList import ItemList, ColumnMapper
from .ContainerPanel import ContainerPanel

class DeploymentCreator(wx.Dialog):
    
    def __init__(self, parent, deploymentTemplate, *args, **kwargs):
        wx.Dialog.__init__(self, parent, *args, **kwargs)
        
        self.deploymentTemplate = deploymentTemplate
        
        self.vsizer = wx.BoxSizer(wx.VERTICAL)
        self.SetSizer(self.vsizer)
                
        self.container = ContainerPanel(self, size=self.GetSize())
        self.vsizer.Add(self.container, 1, wx.ALL|wx.EXPAND)
        
        self.sizePanel = SizePanel(self.container)
        self.container.addPanel("SIZE", self.sizePanel)
        
        self.cloudPanel = CloudPanel(self.container)
        self.container.addPanel("CLOUD", self.cloudPanel)
        
        self.deploymentPanel = DeploymentPanel(self.container, deploymentTemplate)
        self.container.addPanel("DEPLOYMENT", self.deploymentPanel)
        
    def showPanel(self, label):
        self.container.showPanel(label)   
  
  
class SizePanel(wx.Panel):    
    
    def __init__(self, *args, **kwargs):
        
        wx.Panel.__init__(self, *args, **kwargs)
    
        self.chooseButton = wx.Button(self, wx.ID_FORWARD)
        self.cancelButton = wx.Button(self, wx.ID_CANCEL)
        
        self.sizer = wx.BoxSizer(wx.VERTICAL) 
        self.SetSizer(self.sizer)
        
        txt = wx.StaticText(self, -1, "Problem Size")
        txt.SetFont(wx.Font(15, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.sizer.Add(txt, 0, wx.ALL, 2)
        
        self.probSize = wx.TextCtrl(self, -1, size=(100,-1))
        self.sizer.Add(self.probSize, 0, wx.EXPAND|wx.ALL, 5)
       
        txt = wx.StaticText(self, -1, 
        """Enter an integer or floating point value which represents 
the problem size of the deployment's computation. For example, 
if your application's workload is determined by the size of 
a N x N matrix, you should enter N or N^2. Whatever convention 
used, be consistent across all deployment's within a grouping, 
as this will result in better resource usage tracking and 
prediction.""")
        
        self.sizer.Add(txt, 0, wx.ALL, 4)
       
        self.hsizer = wx.BoxSizer(wx.HORIZONTAL)
       
        self.hsizer.Add(self.cancelButton, 0, wx.ALIGN_RIGHT|wx.ALL, 2)  
        self.hsizer.Add(self.chooseButton, 0, wx.ALIGN_RIGHT|wx.ALL, 2)
        
        self.sizer.Add(self.hsizer, 0, wx.ALIGN_RIGHT|wx.ALL, 2) 
        
       
class CloudPanel(wx.Panel):    
    
    def __init__(self, *args, **kwargs):
        
        wx.Panel.__init__(self, *args, **kwargs)
 
        self.cloudList = ItemList(self, -1, style=wx.LC_REPORT, size=(-1, 130),
                                   mappers=[ColumnMapper('Name', lambda c: c.name, defaultWidth=wx.LIST_AUTOSIZE)])
        
        self.chooseButton = wx.Button(self, wx.ID_FORWARD)
        self.cancelButton = wx.Button(self, wx.ID_CANCEL)
        
        self.sizer = wx.BoxSizer(wx.VERTICAL) 
        self.SetSizer(self.sizer)
        
        txt = wx.StaticText(self, -1, 'Choose Cloud')
        txt.SetFont(wx.Font(15, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.sizer.Add(txt, 0, wx.ALL, 2)
        
        self.sizer.Add(self.cloudList, 0, wx.EXPAND|wx.ALL, 5)
       
        self.hsizer = wx.BoxSizer(wx.HORIZONTAL)
       
        self.hsizer.Add(self.cancelButton, 0, wx.ALIGN_RIGHT|wx.ALL, 2)  
        self.hsizer.Add(self.chooseButton, 0, wx.ALIGN_RIGHT|wx.ALL, 2)
        
        self.sizer.Add(self.hsizer, 0, wx.ALIGN_RIGHT|wx.ALL, 2) 
        
class DeploymentPanel(wx.Panel):
    
    def __init__(self, parent, deploymentTemplate):
        wx.Panel.__init__(self, parent)
        
        self.deploymentTemplate = deploymentTemplate
        
        self.vsizer = wx.BoxSizer(wx.VERTICAL)
        self.SetSizer(self.vsizer)
        
        txt = wx.StaticText(self, -1, 'Role Instance Type')
        txt.SetFont(wx.Font(15, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.vsizer.Add(txt, 0, wx.ALL, 2)
        
        self.roleValues = {}
     
        self.doneButton = wx.Button(self, wx.ID_OK)

        self.vsizer.Add(self.doneButton, 0, wx.ALIGN_RIGHT|wx.ALL, 2)
    
        self.choiceIdx = 1
    
    def addRoleChoice(self, choice):
        
        sizer = wx.BoxSizer(wx.HORIZONTAL)
        sizer.Add(wx.StaticText(self, -1, choice.roleTemp.name))
        
        countCtrl = wx.SpinCtrl(self, wx.ID_ANY, '1', min=1, max=1024)
        imgTxt = wx.StaticText(self, wx.ID_ANY, size=(60, -1), label=choice.imgStr)
        instCtrl = wx.ComboBox(self, wx.ID_ANY, size=(120, -1), choices=[c.name for c in choice.instanceTypes])
        
        sizer.Add(countCtrl, 0, wx.ALL, 2)
        sizer.Add(imgTxt, 0, wx.ALL, 2)
        sizer.Add(instCtrl, 0, wx.ALL, 2)
        
        self.vsizer.Insert(self.choiceIdx, sizer)
        
        return (countCtrl, instCtrl)
    
    

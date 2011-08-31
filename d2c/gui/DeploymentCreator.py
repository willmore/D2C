
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
        
        self.credPanel = CredPanel(self.container)
        self.container.addPanel("CREDENTIAL", self.credPanel)
        
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
        
class CredPanel(wx.Panel):
    
    def __init__(self, *args, **kwargs):
        
        wx.Panel.__init__(self, *args, **kwargs);

        self.sizer = wx.BoxSizer(wx.VERTICAL)
        self.SetSizer(self.sizer)
        txt = wx.StaticText(self, -1, "Choose AWS Credential")
        txt.SetFont(wx.Font(15, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.sizer.Add(txt, 0, wx.ALL, 2)
        
        self.credList = ItemList(self, -1, style=wx.LC_REPORT, size=(-1, 150),
                                   mappers=[ColumnMapper('Name', lambda c: c.name, defaultWidth=wx.LIST_AUTOSIZE)])
        self.sizer.Add(self.credList, 0, wx.EXPAND|wx.ALL, 5)
        
        self.chooseButton = wx.Button(self, wx.ID_FORWARD)
        self.cancelButton = wx.Button(self, wx.ID_CANCEL)
        
        self.hsizer = wx.BoxSizer(wx.HORIZONTAL)
       
        self.hsizer.Add(self.cancelButton, 0, wx.ALIGN_RIGHT|wx.ALL, 2)  
        self.hsizer.Add(self.chooseButton, 0, wx.ALIGN_RIGHT|wx.ALL, 2)
        
        self.sizer.Add(self.hsizer, 0, wx.ALIGN_RIGHT|wx.ALL, 2) 
       
class CloudPanel(wx.Panel):    
    
    def __init__(self, *args, **kwargs):
        
        wx.Panel.__init__(self, *args, **kwargs)
 
        self.cloudList = ItemList(self, -1, style=wx.LC_REPORT, size=(-1, 150),
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
        
        staticBox = wx.StaticBox(self, label=choice.roleTemp.name)
        boxSizer = wx.StaticBoxSizer(staticBox, wx.VERTICAL)
       
       
        hboxSizer = wx.BoxSizer(wx.HORIZONTAL)
        countCtrl = wx.SpinCtrl(self, wx.ID_ANY, '1', min=0, max=1024)
        hboxSizer.Add(wx.StaticText(self, -1,"Instance Count"))
        hboxSizer.Add(countCtrl, 0, wx.ALL, 2)
        boxSizer.Add(hboxSizer, 1)
                
        hboxSizer = wx.BoxSizer(wx.HORIZONTAL)
        imgTxt = wx.StaticText(self, wx.ID_ANY, label=choice.imgStr)
        hboxSizer.Add(wx.StaticText(self, -1,"Image"), 0)
        hboxSizer.Add(imgTxt, 1)
        boxSizer.Add(hboxSizer, 1)
        
        hboxSizer = wx.BoxSizer(wx.HORIZONTAL)
        instCtrl = wx.ComboBox(self, wx.ID_ANY, size=(120, -1), choices=[c.name for c in choice.instanceTypes])
        hboxSizer.Add(wx.StaticText(self, -1,"Instance Type"))
        hboxSizer.Add(instCtrl, 0, wx.ALL, 2)
        boxSizer.Add(hboxSizer, 1)
        
        self.vsizer.Insert(self.choiceIdx, boxSizer, 0, wx.EXPAND | wx.ALL, 2)
        
        return (countCtrl, instCtrl)
    
    


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
        
        self.cloudPanel = CloudPanel(self.container)
        self.container.addPanel("CLOUD", self.cloudPanel)
        
        self.deploymentPanel = DeploymentPanel(self.container, deploymentTemplate)
        self.container.addPanel("DEPLOYMENT", self.deploymentPanel)
        
    def showPanel(self, label):
        self.container.showPanel(label)   
       
class CloudPanel(wx.Panel):    
    
    def __init__(self, *args, **kwargs):
        
        wx.Panel.__init__(self, *args, **kwargs)
 
        self.cloudList = ItemList(self, -1, style=wx.LC_REPORT, size=(-1, 130),
                                   mappers=[ColumnMapper('Name', lambda c: c.name, defaultWidth=wx.LIST_AUTOSIZE)])
        
        self.chooseButton = wx.Button(self, wx.ID_FORWARD)
        #self.cancelButton = wx.Button(self, wx.ID_ANY, 'Cancel')
        self.cancelButton = wx.Button(self, wx.ID_CANCEL)
        
        self.sizer = wx.BoxSizer(wx.VERTICAL) 
        self.SetSizer(self.sizer)
        
        addStoreTxt = wx.StaticText(self, -1, 'Choose Cloud')
        addStoreTxt.SetFont(wx.Font(15, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.sizer.Add(addStoreTxt, 0)
        
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
        
        self.vsizer.Add(wx.StaticText(self, -1, 'Specify role attributes:'))
        
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
    
    

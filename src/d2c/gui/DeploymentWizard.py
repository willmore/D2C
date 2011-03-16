import wx
from .ContainerPanel import ContainerPanel
from .AMIList import AMIList
from .RoleList import RoleList

class AMISelectorPanel(wx.Panel):
    def __init__(self, parent, id=-1, size=wx.DefaultSize):
        wx.Panel.__init__(self, parent, id=id, size=size)
        self.sizer = wx.BoxSizer(wx.VERTICAL)
        self.SetSizer(self.sizer)
        
        self.amiList = wx.ListBox(self, -1, wx.DefaultPosition, (170, 130), ("foo","bar","baz"), wx.LB_SINGLE)
        self.sizer.Add(self.amiList, 0, wx.ALL|wx.EXPAND)
        
class RolePanel(wx.Panel):
    def __init__(self, parent, id=-1, size=wx.DefaultSize):
        wx.Panel.__init__(self, parent, id=id, size=size)
        self.sizer = wx.BoxSizer(wx.VERTICAL)
        self.SetSizer(self.sizer)
        
        self.roleList = RoleList(self, -1, style=wx.LC_REPORT)

        self.sizer.Add(self.roleList, 0, wx.ALL|wx.EXPAND)
        
        self.addRoleButton = wx.Button(self, wx.ID_ANY, 'Add New Role', size=(110, -1))
        self.sizer.Add(self.addRoleButton, 0, wx.ALIGN_RIGHT)
        
        self.nextButton = wx.Button(self, wx.ID_ANY, 'Next', size=(110, -1))
        self.sizer.Add(self.nextButton, 0, wx.ALIGN_RIGHT)

class AddRolePanel(wx.Panel):
    
    def __init__(self, *args, **kwargs):
        wx.Panel.__init__(self, *args, **kwargs)
        
        self.sizer = wx.BoxSizer(wx.VERTICAL)
       
        self.sizer.Add(wx.StaticText(self, -1, 'Add Role'), 0)
        
        self.amiList = AMIList(self, -1, style=wx.LC_REPORT)
        self.sizer.Add(self.amiList, 1, wx.EXPAND)
        
        self.roleName = wx.TextCtrl(self);
        self.hostCount = wx.TextCtrl(self);
        
        fgs = wx.FlexGridSizer(7,2,0,0)
        fgs.AddGrowableCol(1, 1)
        
        fgs.AddMany([   (wx.StaticText(self, -1, 'Role Name'),0, wx.ALIGN_RIGHT),
                        (self.roleName, 0, wx.ALIGN_LEFT|wx.ALIGN_CENTER_VERTICAL|wx.EXPAND),
                        
                        (wx.StaticText(self, -1, 'Host Count'),0, wx.ALIGN_RIGHT),
                        (self.hostCount, 0, wx.ALIGN_LEFT|wx.ALIGN_CENTER_VERTICAL|wx.EXPAND)
                           ])
        
        self.sizer.Add(fgs, 0, wx.ALL | wx.EXPAND)
        
        self.addRoleButton = wx.Button(self, wx.ID_ANY, 'Add New Role', size=(110, -1))
        self.sizer.Add(self.addRoleButton, 0, wx.ALIGN_RIGHT)
        
        self.SetSizer(self.sizer)

class RoleWizardContainer(ContainerPanel):
    
    def __init__(self, *args, **kwargs):
        ContainerPanel.__init__(self, *args, **kwargs)
          
        self.addPanel("ROLES", RolePanel(self))  
        self.addPanel("ADD_ROLE", AddRolePanel(self))      
        
        self.showPanel("ROLES")


class SettingsPanel(wx.Panel):    
    
    def __init__(self, *args, **kwargs):
        wx.Panel.__init__(self, *args, **kwargs)
      
        self.startScript = wx.TextCtrl(self);
        self.endCheck = wx.TextCtrl(self);
        self.data = wx.TextCtrl(self);
        vbox = wx.BoxSizer(wx.VERTICAL)
        fgs = wx.FlexGridSizer(7,2,0,0)
        fgs.AddGrowableCol(1, 1)
        
        fgs.AddMany([   (wx.StaticText(self, -1, 'Start Script'),0, wx.ALIGN_RIGHT),
                        (self.startScript, 0, wx.ALIGN_LEFT|wx.ALIGN_CENTER_VERTICAL|wx.EXPAND),
                        
                        (wx.StaticText(self, -1, 'End Check'),0, wx.ALIGN_RIGHT),
                        (self.endCheck, 0, wx.ALIGN_LEFT|wx.ALIGN_CENTER_VERTICAL|wx.EXPAND),
                     
                           (wx.StaticText(self, -1, 'Data to Collect'),0, wx.ALIGN_RIGHT),
                           (self.data, 0, wx.ALIGN_LEFT|wx.ALIGN_CENTER_VERTICAL|wx.EXPAND)
                           
                           ])
        
        vbox.Add(fgs, 0, wx.ALL | wx.EXPAND)
        
        self.finishButton = wx.Button(self, wx.ID_ANY, 'Finish', size=(110, -1))
        vbox.Add(self.finishButton, 0, wx.ALIGN_RIGHT)
        
        self.SetSizer(vbox)
        
class CompletionPanel(wx.Panel):
    
    def __init__(self, *args, **kwargs):
        wx.Panel.__init__(self, *args, **kwargs)
        
        self.sizer = wx.BoxSizer(wx.VERTICAL)
       
        self.sizer.Add(wx.StaticText(self, -1, 'New completion created!'), 0)
        
        self.okButton = wx.Button(self, wx.ID_ANY, 'OK', size=(110, -1))
        self.sizer.Add(self.okButton, 0, wx.ALIGN_RIGHT)
        
        self.SetSizer(self.sizer)

class DeploymentWizard(wx.Dialog):
    
    def __init__(self, parent, id, title):
        wx.Dialog.__init__(self, parent, id, title)
        
        self.vsizer = wx.BoxSizer(wx.VERTICAL)
        self.SetSizer(self.vsizer)
                
        self.container = ContainerPanel(self)
        self.vsizer.Add(self.container, 0, wx.ALL|wx.EXPAND)
           
        self.roleWizard = RoleWizardContainer(self.container)
        self.container.addPanel("ROLES", self.roleWizard)   
        
        self.settingsPanel = SettingsPanel(self.container)
        self.container.addPanel("CONF", self.settingsPanel)    
        
        self.completionPanel = CompletionPanel(self.container)
        self.container.addPanel("COMPLETION", self.completionPanel)        
        

    
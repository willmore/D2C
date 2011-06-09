import wx
from .ContainerPanel import ContainerPanel
from .AMIList import AMIList
from .RoleList import RoleList

        
class RolePanel(wx.Panel):
    def __init__(self, *args, **kwargs):
        wx.Panel.__init__(self, *args, **kwargs)
        self.sizer = wx.BoxSizer(wx.VERTICAL)
        
        self.roleList = RoleList(self, -1, style=wx.LC_REPORT)

        self.sizer.Add(self.roleList, 0, wx.EXPAND|wx.ALL, 5)
        
        self.addRoleButton = wx.Button(self, wx.ID_ANY, 'Add New Role', size=(110, -1))
        self.sizer.Add(self.addRoleButton, 0, wx.ALIGN_RIGHT|wx.ALL, 2)
        
        self.finishButton = wx.Button(self, wx.ID_ANY, 'Finish', size=(110, -1))
        self.sizer.Add(self.finishButton, 0, wx.ALIGN_RIGHT|wx.ALL, 2)
        
        self.SetSizer(self.sizer)

class AddRolePanel(wx.Panel):
    
    def __init__(self, *args, **kwargs):
        wx.Panel.__init__(self, *args, **kwargs)
        
        self.sizer = wx.BoxSizer(wx.VERTICAL)
        addRoleTxt = wx.StaticText(self, -1, 'Add Role')
        addRoleTxt.SetFont(wx.Font(15, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.sizer.Add(addRoleTxt, 0)
        
        self.amiList = AMIList(self, -1, style=wx.LC_REPORT)
        self.sizer.Add(self.amiList, 1, wx.EXPAND | wx.ALL, 5)
        
        self.roleName = wx.TextCtrl(self)
        
        self.hostCount = wx.SpinCtrl(self, size=(60, -1))
        self.hostCount.SetRange(1, 1000)
        
        self.instanceType = wx.TextCtrl(self)
        self.startScript = wx.TextCtrl(self)
        self.endCheck = wx.TextCtrl(self)
        self.data = wx.TextCtrl(self);
        
        fgs = wx.FlexGridSizer(7,2,0,0)
        fgs.AddGrowableCol(1, 1)
        
        fgs.AddMany([   (wx.StaticText(self, -1, 'Role Name'),0, wx.ALIGN_RIGHT|wx.ALL, 2),
                        (self.roleName, 0, wx.ALIGN_LEFT|wx.ALIGN_CENTER_VERTICAL|wx.EXPAND|wx.ALL, 2),
                        
                        (wx.StaticText(self, -1, 'Host Count'),0, wx.ALIGN_RIGHT|wx.ALL, 2),
                        (self.hostCount, 0, wx.ALIGN_LEFT|wx.ALIGN_CENTER_VERTICAL|wx.ALL, 2),
                        
                        (wx.StaticText(self, -1, 'Instance Type'),0, wx.ALIGN_RIGHT|wx.ALL, 2),
                        (self.instanceType, 0, wx.ALIGN_LEFT|wx.ALIGN_CENTER_VERTICAL|wx.EXPAND|wx.ALL, 2),
                        (wx.StaticText(self, -1, 'Start Script'),0, wx.ALIGN_RIGHT|wx.ALL, 2),
                        (self.startScript, 0, wx.ALIGN_LEFT|wx.ALIGN_CENTER_VERTICAL|wx.EXPAND|wx.ALL, 2),
                        
                        (wx.StaticText(self, -1, 'End Check'),0, wx.ALIGN_RIGHT|wx.ALL, 2),
                        (self.endCheck, 0, wx.ALIGN_LEFT|wx.ALIGN_CENTER_VERTICAL|wx.EXPAND|wx.ALL, 2),
                     
                        (wx.StaticText(self, -1, 'Data to Collect'),0, wx.ALIGN_RIGHT|wx.ALL, 2),
                        (self.data, 0, wx.ALIGN_LEFT|wx.ALIGN_CENTER_VERTICAL|wx.EXPAND|wx.ALL, 2)
                           ])
        
        self.sizer.Add(fgs, 0, wx.ALL | wx.EXPAND, 5)
        
        self.addRoleButton = wx.Button(self, wx.ID_ANY, 'Add New Role', size=(110, -1))
        self.sizer.Add(self.addRoleButton, 0, wx.ALIGN_RIGHT | wx.ALL, 5)
        
        self.SetSizer(self.sizer)

class RoleWizardContainer(ContainerPanel):
    
    def __init__(self, *args, **kwargs):
        ContainerPanel.__init__(self, *args, **kwargs)
          
        self.addPanel("ROLES", RolePanel(self))  
        self.addPanel("ADD_ROLE", AddRolePanel(self))      
        
        self.showPanel("ROLES")


        
class CompletionPanel(wx.Panel):
    
    def __init__(self, *args, **kwargs):
        wx.Panel.__init__(self, *args, **kwargs)
        
        self.sizer = wx.BoxSizer(wx.VERTICAL)
       
        self.sizer.Add(wx.StaticText(self, -1, 'New completion created!'), 0)
       
        self.okButton = wx.Button(self, wx.ID_ANY, 'OK!', size=(110, -1))
        self.sizer.Add(self.okButton, 0, wx.ALIGN_RIGHT)
        
        self.SetSizer(self.sizer)
        
        
class NamePanel(wx.Panel):
    
    def __init__(self, *args, **kwargs):
        wx.Panel.__init__(self, *args, **kwargs)
        
        self.sizer = wx.BoxSizer(wx.VERTICAL)
       
        hbox = wx.BoxSizer(wx.HORIZONTAL)
        hbox.Add(wx.StaticText(self, -1, 'Please enter the name of the deployment'))
        self.sizer.Add(hbox, 0, wx.ALL, 5)
        
        hbox = wx.BoxSizer(wx.HORIZONTAL)
        self.name = wx.TextCtrl(self, -1,size=(200,-1))
        hbox.Add(self.name) 
        self.sizer.Add(hbox, 0, wx.ALL, 5)
        
        
        self.nextButton = wx.Button(self, wx.ID_ANY, 'Next', size=(110, -1))
        
        hbox = wx.BoxSizer(wx.HORIZONTAL)
        hbox.Add(self.nextButton, 0, wx.ALIGN_RIGHT|wx.ALL, 5)
        self.sizer.Add(hbox, 0, wx.ALIGN_RIGHT)
        
        self.SetSizer(self.sizer)

class DeploymentWizard(wx.Dialog):
    
    def __init__(self, *args, **kwargs):
        wx.Dialog.__init__(self, *args, **kwargs)
        
        self.vsizer = wx.BoxSizer(wx.VERTICAL)
        self.SetSizer(self.vsizer)
                
        self.container = ContainerPanel(self, size=self.GetSize())
        self.vsizer.Add(self.container, 1, wx.ALL|wx.EXPAND)
        
        self.namePanel = NamePanel(self.container)
        self.container.addPanel("NAME", self.namePanel)  
         
        self.roleWizard = RoleWizardContainer(self.container)
        self.container.addPanel("ROLES", self.roleWizard)   
        
        self.completionPanel = CompletionPanel(self.container)
        self.container.addPanel("COMPLETION", self.completionPanel)        
        
    
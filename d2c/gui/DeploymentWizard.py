import wx
from .ContainerPanel import ContainerPanel
from .AMIList import AMIList
from .RoleList import RoleList

        
class RolePanel(wx.Panel):
    def __init__(self, *args, **kwargs):
        wx.Panel.__init__(self, *args, **kwargs)

        self.sizer = wx.BoxSizer(wx.VERTICAL)
        
        self.rolesText= wx.StaticText(self, -1, 'Roles:')
        
        self.sizer.Add(self.rolesText,0,wx.ALIGN_LEFT|wx.ALL,5)
        
        self.roleList = RoleList(self, -1, style=wx.LC_REPORT)

        self.sizer.Add(self.roleList, 0, wx.EXPAND|wx.ALL, 5)
        
        self.hsizer = wx.BoxSizer(wx.HORIZONTAL)
        
        self.addRoleButton = wx.Button(self, wx.ID_ANY, 'Add New Role')
        self.hsizer.Add(self.addRoleButton, 0, wx.ALIGN_LEFT|wx.ALL, 2)
        
        
        
        self.backButton = wx.Button(self, wx.ID_ANY, 'Back')
        self.hsizer.Add(self.backButton,0, wx.ALIGN_RIGHT|wx.ALL, 2)
        
        self.finishButton = wx.Button(self, wx.ID_ANY, 'Finish')
        self.hsizer.Add(self.finishButton,0, wx.ALIGN_RIGHT|wx.ALL, 2)
        
        self.sizer.Add(self.hsizer,0, wx.ALIGN_RIGHT)
        
        self.SetSizer(self.sizer)
        self.Layout()

class AddRolePanel(wx.Panel):
    
    def __init__(self, *args, **kwargs):
        wx.Panel.__init__(self, *args, **kwargs)
        
        self.sizer = wx.BoxSizer(wx.VERTICAL)
        addRoleTxt = wx.StaticText(self, -1, 'Add Role')
        addRoleTxt.SetFont(wx.Font(15, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.sizer.Add(addRoleTxt, 0)
        
        self.amiList = AMIList(self, -1, style=wx.LC_REPORT)
        self.sizer.Add(self.amiList, 0, wx.EXPAND | wx.ALL, 5)
        
        self.roleName = wx.TextCtrl(self)
        
        self.hostCount = wx.SpinCtrl(self, size=(60, -1))
        self.hostCount.SetRange(1, 1000)
        
        self.instanceType = wx.ComboBox(self, style=wx.CB_READONLY)
        
        self.fgs = wx.FlexGridSizer(3,2,0,0)
        self.fgs.AddGrowableCol(1, 1)
        self.fgs.AddMany([
                        (wx.StaticText(self, -1, 'Role Name'),0, wx.ALIGN_RIGHT|wx.ALL, 2),
                        (self.roleName, 0, wx.ALIGN_LEFT|wx.ALIGN_CENTER_VERTICAL|wx.EXPAND|wx.ALL, 2),
                        
                        (wx.StaticText(self, -1, 'Host Count'),0, wx.ALIGN_RIGHT|wx.ALL, 2),
                        (self.hostCount, 0, wx.ALIGN_LEFT|wx.ALIGN_CENTER_VERTICAL|wx.ALL, 2),
                         
                        (wx.StaticText(self, -1, 'Instance Type'),0, wx.ALIGN_RIGHT|wx.ALL, 2),
                        (self.instanceType, 0, wx.ALIGN_LEFT|wx.ALIGN_CENTER_VERTICAL|wx.EXPAND|wx.ALL, 2)]
                       )
        
        self.sizer.Add(self.fgs, 0, wx.ALL | wx.EXPAND, 5)
        
        
        
        self.sw = wx.ScrolledWindow(self, style=wx.VSCROLL)
        self.sw.SetMinSize((-1, 300))
        self.SetScrollbar(1,1,1,1)
        self.sizer.Add(self.sw, 0, wx.ALL | wx.EXPAND, 5)
        self.sw.sizer = wx.BoxSizer(wx.VERTICAL)
        
        self.sw.uploadScriptBox = wx.StaticBox(self.sw, label="Uploads")
        self.sw.uploadScriptBox.boxSizer = wx.StaticBoxSizer(self.sw.uploadScriptBox, wx.VERTICAL)
        
        self.sw.sizer.Add(self.sw.uploadScriptBox.boxSizer, 0, wx.EXPAND| wx.ALL, 2)
        ###
        
        self.sw.startScriptBox = wx.StaticBox(self.sw, label="Start Scripts")
        self.sw.startScriptBox.boxSizer = wx.StaticBoxSizer(self.sw.startScriptBox, wx.VERTICAL)
        
        self.sw.sizer.Add(self.sw.startScriptBox.boxSizer, 0, wx.EXPAND| wx.ALL, 2)
        
        #####
        self.sw.endScriptBox = wx.StaticBox(self.sw, label="File Done Check")
        self.sw.endScriptBox.boxSizer = wx.StaticBoxSizer(self.sw.endScriptBox, wx.VERTICAL)
        self.sw.sizer.Add(self.sw.endScriptBox.boxSizer, 0, wx.EXPAND| wx.ALL, 2)
        
        ####
        self.sw.dataBox = wx.StaticBox(self.sw, label="Data to Collect")
        self.sw.dataBox.boxSizer = wx.StaticBoxSizer(self.sw.dataBox, wx.VERTICAL)      
        self.sw.sizer.Add(self.sw.dataBox.boxSizer, 0, wx.EXPAND | wx.ALL, 2)
        
        
        self.sw.SetSizer(self.sw.sizer)
        
        self.hsizer = wx.BoxSizer(wx.HORIZONTAL)
        
        self.addRoleButton = wx.Button(self, wx.ID_ANY, 'Add New Role')
        self.hsizer.Add(self.addRoleButton,0, wx.ALIGN_RIGHT)
        
        self.cancelButton = wx.Button(self, wx.ID_ANY, 'Cancel')
        self.hsizer.Add(self.cancelButton,wx.ALIGN_RIGHT)
        
        self.sizer.Add(self.hsizer, 0, wx.ALIGN_RIGHT|wx.ALL, 2)
        
        self.SetSizer(self.sizer)
        self.Layout()

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
        self.nextButton.Disable()
        
        hbox = wx.BoxSizer(wx.HORIZONTAL)
        hbox.Add(self.nextButton, 0, wx.ALIGN_RIGHT|wx.ALL, 5)
        self.sizer.Add(hbox, 0, wx.ALIGN_RIGHT)
        
        self.SetSizer(self.sizer)
        
class CloudPanel(wx.Panel):
    
    def __init__(self, *args, **kwargs):
        wx.Panel.__init__(self, *args, **kwargs)
        
        self.sizer = wx.BoxSizer(wx.VERTICAL)
       
        hbox = wx.BoxSizer(wx.HORIZONTAL)
        hbox.Add(wx.StaticText(self, -1, 'Choose a cloud to deploy on'))
        self.sizer.Add(hbox, 0, wx.ALL, 5)
        
        hbox = wx.BoxSizer(wx.HORIZONTAL)
        self.clouds = wx.ComboBox(self, -1,style=wx.CB_READONLY)
        hbox.Add(self.clouds) 
        self.sizer.Add(hbox, 0, wx.ALL, 5)
        
        self.backButton = wx.Button(self, wx.ID_ANY, 'Back', size=(110, -1))
        self.nextButton = wx.Button(self, wx.ID_ANY, 'Next', size=(110, -1))
        self.nextButton.Disable()
        
        
        hbox = wx.BoxSizer(wx.HORIZONTAL)
        hbox.Add(self.backButton, 0, wx.ALIGN_RIGHT|wx.ALL, 5)
        hbox.Add(self.nextButton, 0, wx.ALIGN_RIGHT|wx.ALL, 5)
        
        self.sizer.Add(hbox, 0, wx.ALIGN_RIGHT)
        
        self.SetSizer(self.sizer)


class DeploymentWizard(wx.Dialog):
    
    def __init__(self, *args, **kwargs):
        wx.Dialog.__init__(self, size=(400, 300), *args, **kwargs)
        
        self.vsizer = wx.BoxSizer(wx.VERTICAL)
        self.SetSizer(self.vsizer)
                
        self.container = ContainerPanel(self, size=self.GetSize())
        self.vsizer.Add(self.container, 1, wx.ALL|wx.EXPAND)
        
        self.namePanel = NamePanel(self.container)
        self.container.addPanel("NAME", self.namePanel)  
        
        self.cloudPanel = CloudPanel(self.container)
        self.container.addPanel("CLOUD", self.cloudPanel) 
         
        self.roleWizard = RoleWizardContainer(self.container)
        self.container.addPanel("ROLES", self.roleWizard)   
        
        self.completionPanel = CompletionPanel(self.container)
        self.container.addPanel("COMPLETION", self.completionPanel)        
        
    
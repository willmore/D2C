import wx
from .ContainerPanel import ContainerPanel
from .ItemList import ItemList, ColumnMapper
  
       
class CloudPanel(wx.Panel):    
    
    def __init__(self, *args, **kwargs):
        
        wx.Panel.__init__(self, *args, **kwargs)
 
        self.cloudList = ItemList(self, -1, style=wx.LC_REPORT, size=(-1, 100),
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
        
        self.credCombo = wx.ComboBox(self, -1, style=wx.CB_READONLY) 
        self.sizer.Add(self.credCombo, 0, wx.ALL, 5)
        
        self.hsizer = wx.BoxSizer(wx.HORIZONTAL)
       
        self.hsizer.Add(self.cancelButton, 0, wx.ALIGN_RIGHT|wx.ALL, 2)  
        self.hsizer.Add(self.chooseButton, 0, wx.ALIGN_RIGHT|wx.ALL, 2)
        
        self.sizer.Add(self.hsizer, 0, wx.ALIGN_RIGHT|wx.ALL, 2)   
        
        
        
    def setClouds(self, clouds):
        self.cloudList.setItems(clouds)
    
    def setCloudCreds(self, creds):
        self.credCombo.SetItems(creds)
        
class KernelPanel(wx.Panel):    
    
    def __init__(self, *args, **kwargs):
        wx.Panel.__init__(self, *args, **kwargs)
 
        self.kernelList = ItemList(self, -1, style=wx.LC_REPORT, size=(-1, 100),
                                     mappers=[ColumnMapper('AKI', lambda k: k.aki, defaultWidth=90),
                                              ColumnMapper('Architecture', lambda k: k.architecture.arch, defaultWidth=110)])
        self.chooseButton = wx.Button(self, wx.ID_ANY, 'Next')
        self.backButton = wx.Button(self, wx.ID_ANY, 'Back')
        
        self.sizer = wx.BoxSizer(wx.VERTICAL) 
        self.SetSizer(self.sizer)
        
        addStoreTxt = wx.StaticText(self, -1, 'Choose Kernel')
        addStoreTxt.SetFont(wx.Font(15, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.sizer.Add(addStoreTxt, 0)
        
        self.sizer.Add(self.kernelList, 0, wx.EXPAND|wx.ALL, 5)
        
        self.hsizer = wx.BoxSizer(wx.HORIZONTAL)
        
        self.hsizer.Add(self.backButton, 0, wx.ALIGN_RIGHT|wx.ALL, 2)
        self.hsizer.Add(self.chooseButton, 0, wx.ALIGN_RIGHT|wx.ALL, 2)
             
        self.sizer.Add(self.hsizer, 0, wx.ALIGN_RIGHT|wx.ALL, 2)
        
        
class RamdiskPanel(wx.Panel):    
    
    def __init__(self, *args, **kwargs):
        wx.Panel.__init__(self, *args, **kwargs)
 
        self.rdList = ItemList(self, -1, style=wx.LC_REPORT, size=(-1, 100),
                                     mappers=[ColumnMapper('ARI', lambda k: k.id, defaultWidth=90)])
        self.chooseButton = wx.Button(self, wx.ID_ANY, 'Next')
        self.backButton = wx.Button(self, wx.ID_ANY, 'Back')
        
        self.sizer = wx.BoxSizer(wx.VERTICAL) 
        self.SetSizer(self.sizer)
        
        addStoreTxt = wx.StaticText(self, -1, 'Choose Ramdisk')
        addStoreTxt.SetFont(wx.Font(15, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.sizer.Add(addStoreTxt, 0)
        
        self.sizer.Add(self.rdList, 0, wx.EXPAND|wx.ALL, 5)
        
        self.hsizer = wx.BoxSizer(wx.HORIZONTAL)
        
        self.hsizer.Add(self.backButton, 0, wx.ALIGN_RIGHT|wx.ALL, 2)
        self.hsizer.Add(self.chooseButton, 0, wx.ALIGN_RIGHT|wx.ALL, 2)
             
        self.sizer.Add(self.hsizer, 0, wx.ALIGN_RIGHT|wx.ALL, 2)
        
class BucketPanel(wx.Panel):    
    
    def __init__(self, *args, **kwargs):
        wx.Panel.__init__(self, *args, **kwargs)
 
        self.createButton = wx.Button(self, wx.ID_ANY, 'Create AMI', size=(190, -1))
        self.cancelButton = wx.Button(self, wx.ID_ANY, 'Cancel', size=(190, -1))
        
        self.sizer = wx.BoxSizer(wx.VERTICAL) 
        self.SetSizer(self.sizer)
        
        addStoreTxt = wx.StaticText(self, -1, 'Choose bucket to store image')
        addStoreTxt.SetFont(wx.Font(15, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.sizer.Add(addStoreTxt, 0)
        
        self.bucket = wx.TextCtrl(self)
        
        hbox = wx.BoxSizer(wx.HORIZONTAL)
        hbox.Add(wx.StaticText(self, -1, 'Choose bucket to store image'), 0)
        hbox.Add(self.bucket, 1)
        self.sizer.Add(hbox, 0, wx.ALL|wx.EXPAND, 2)
        self.sizer.Add(self.createButton, 0, wx.ALIGN_RIGHT|wx.ALL, 2)
        self.sizer.Add(self.cancelButton, 0, wx.ALIGN_RIGHT|wx.ALL, 2)     


class NewAMIWizard(wx.Dialog):
    
    def __init__(self, *args, **kwargs):
        wx.Dialog.__init__(self, *args, **kwargs)
        
        self.vsizer = wx.BoxSizer(wx.VERTICAL)
        self.SetSizer(self.vsizer)
                
        self.container = ContainerPanel(self, size=self.GetSize())
        self.vsizer.Add(self.container, 1, wx.ALL|wx.EXPAND)
        
        self.cloudPanel = CloudPanel(self.container)
        self.container.addPanel("CLOUD", self.cloudPanel)
        
        self.kernelPanel = KernelPanel(self.container)
        self.container.addPanel("KERNEL", self.kernelPanel)
        
        self.ramdiskPanel = RamdiskPanel(self.container)
        self.container.addPanel("RAMDISK", self.ramdiskPanel)
        
        self.bucketPanel = BucketPanel(self.container)
        self.container.addPanel("BUCKET", self.bucketPanel)
        
    def showPanel(self, label):
        self.container.showPanel(label)
        
        
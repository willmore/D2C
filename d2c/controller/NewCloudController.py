import wx
from d2c.model.Cloud import Cloud
from d2c.model.Kernel import Kernel

def fieldsNotEmpty(self, *fields):
        for f in fields:
            if len(f.GetValue()) == 0:
                return False
            
        return True

class NewCloudController:
    
    def __init__(self, view, dao):
        
        self._view = view
        self._dao = dao
         
        self._view.container.getPanel("MAIN").addButton.Bind(wx.EVT_BUTTON, self.createCloud)
        
        self._view.container.getPanel("NEW_CLOUD").name.Bind(wx.EVT_TEXT, self.checkCanSaveCloud)
        self._view.container.getPanel("NEW_CLOUD").serviceURL.Bind(wx.EVT_TEXT, self.checkCanSaveCloud)
        self._view.container.getPanel("NEW_CLOUD").storageURL.Bind(wx.EVT_TEXT, self.checkCanSaveCloud)
        self._view.container.getPanel("NEW_CLOUD").ec2Cert.Bind(wx.EVT_TEXT, self.checkCanSaveCloud)
        
        self._view.container.getPanel("NEW_CLOUD").kernelId.Bind(wx.EVT_TEXT, self.checkCanAddKernel)
        self._view.container.getPanel("NEW_CLOUD").kernelData.Bind(wx.EVT_TEXT, self.checkCanAddKernel)
        
        self._view.container.getPanel("NEW_CLOUD").addKernelButton.Bind(wx.EVT_BUTTON, self.addKernel)
        self._view.container.getPanel("NEW_CLOUD").saveButton.Bind(wx.EVT_BUTTON, self.save)
        
        self._view.container.showPanel("MAIN")
    
    def save(self, _):
        
        panel = self._view.container.getPanel("NEW_CLOUD")
        
        cloud = Cloud(panel.name.GetValue(), panel.serviceURL.GetValue(), 
                      panel.storageURL.GetValue(), panel.ec2Cert.GetValue(), 
                      panel.kernelList.getItems())
        
        self._dao.saveCloud(cloud)
        self._view.container.getPanel("MAIN").cloudList.addItem(cloud)
        self._view.container.showPanel("MAIN")
        
    
    def addKernel(self, _):
        panel = self._view.container.getPanel("NEW_CLOUD")
        panel.kernelList.addItem(Kernel(panel.kernelId.GetValue(), panel.kernelId.GetValue(), 
                                        Kernel.ARCH_X86_64, panel.kernelData.GetValue()))
        
        
    def checkCanAddKernel(self, _):
        
        panel = self._view.container.getPanel("NEW_CLOUD")
        
        if fieldsNotEmpty(panel.kernelId,
                          panel.kernelData):
            panel.addKernelButton.Enable() 
        else:
            panel.addKernelButton.Disable()
            
    def checkCanSaveCloud(self, _):
        panel = self._view.container.getPanel("NEW_CLOUD")
        
        if fieldsNotEmpty(panel.name, panel.serviceURL, 
                          panel.storageURL, panel.ec2Cert):
            panel.saveButton.Enable()
        else:
            panel.saveButton.Disable()
            
    
    def createCloud(self, _):
        self._view.container.getPanel("NEW_CLOUD").clear()
        self._view.container.showPanel("NEW_CLOUD")
    
    def addStore(self, _):
        
        newStorePanel = self._view.container.getPanel("NEW_STORE")
        try:
            store = newStorePanel.getStore()
        except Exception as x:
            wx.MessageBox(x.message, 'Info')
            return
        
        newStorePanel.clear()
        
        self._dao.addImageStore(store)
        self._view.container.getPanel("MAIN").addStore(store)
        self._view.showPanel("MAIN")
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
         
        self._view.container.getPanel("MAIN").cloudList.setItems(self._dao.getClouds()) 
         
        self._view.container.getPanel("MAIN").doneButton.Bind(wx.EVT_BUTTON, self.done)
        self._view.container.getPanel("MAIN").addButton.Bind(wx.EVT_BUTTON, self.showAddCloud)
        
        
        self._view.container.getPanel("NEW_CLOUD").name.Bind(wx.EVT_TEXT, self.checkCanSaveCloud)
        self._view.container.getPanel("NEW_CLOUD").serviceURL.Bind(wx.EVT_TEXT, self.checkCanSaveCloud)
        self._view.container.getPanel("NEW_CLOUD").storageURL.Bind(wx.EVT_TEXT, self.checkCanSaveCloud)
        self._view.container.getPanel("NEW_CLOUD").ec2Cert.Bind(wx.EVT_TEXT, self.checkCanSaveCloud)
        
        self._view.container.getPanel("NEW_CLOUD").kernelId.Bind(wx.EVT_TEXT, self.checkCanAddKernel)
        self._view.container.getPanel("NEW_CLOUD").kernelData.Bind(wx.EVT_TEXT, self.checkCanAddKernel)
        
        self._view.container.getPanel("NEW_CLOUD").addKernelButton.Bind(wx.EVT_BUTTON, self.addKernel)
        self._view.container.getPanel("NEW_CLOUD").saveButton.Bind(wx.EVT_BUTTON, self.save)
        self._view.container.getPanel("NEW_CLOUD").closeButton.Bind(wx.EVT_BUTTON, self.close)
        
        
        self._view.container.showPanel("MAIN")
    
    def showAddCloud(self, _):
        self._view.container.showPanel("NEW_CLOUD")
    
    def done(self, _):
        self._view.EndModal(wx.ID_OK)
        
    def close(self,_):
        self._view.container.showPanel("MAIN")
    
    def save(self, _):
        
        panel = self._view.container.getPanel("NEW_CLOUD")
        
        cloud = Cloud(panel.name.GetValue(), panel.serviceURL.GetValue(), 
                      panel.storageURL.GetValue(), panel.ec2Cert.GetValue())
        
        cloud.kernels = panel.kernelList.getItems()
        
        self._dao.saveCloud(cloud)
        self._view.container.getPanel("MAIN").cloudList.addItem(cloud)
        self._view.container.showPanel("MAIN")
        
    
    def addKernel(self, _):
        panel = self._view.container.getPanel("NEW_CLOUD")
        panel.kernelList.addItem(Kernel(panel.kernelId.GetValue(), 
                                        self._dao.getArchitecture('x86_64'), panel.kernelData.GetValue()))
        
        
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
            
    
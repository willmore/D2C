import wx
from wx.lib.pubsub import Publisher

class AMIWizardController:
    
    def __init__(self, view, dao):
        
        self._view = view
        self._dao = dao
         
        self._view.container.getPanel("CLOUD").setClouds(dao.getRegions())
        self._view.container.getPanel("CLOUD").chooseButton.Bind(wx.EVT_BUTTON, self.selectCloud)
        self._view.container.getPanel("CLOUD").cancelButton.Bind(wx.EVT_BUTTON, lambda _: self._view.EndModal(wx.ID_OK))
        self._view.showPanel("CLOUD") 
        
        self._view.container.getPanel("KERNEL").chooseButton.Bind(wx.EVT_BUTTON, self.createAMI)
        
    def showPanel(self, label):
        return lambda _: self._view.showPanel(label)
    
    class Message:
        
        def __init__(self, img, store):
            self.img = img
            self.store = store
            
    def selectCloud(self, _):
        regions = self._view.container.getPanel("CLOUD").regionList.getSelectedRegions()
        
        if len(regions) != 1:
            wx.MessageBox("Select one Cloud.", 'Info')
            return
            
        self.region = regions[0]
        self._view.container.getPanel("KERNEL").kernelList.setKernels(self.region.getKernels())
        self._view.showPanel("KERNEL")
    
    def createAMI(self, _):
        
        kernelPanel = self._view.container.getPanel("KERNEL")
        try:
            kernels = kernelPanel.storeList.getSelectedStores()
            if len(kernels) != 1:
                wx.MessageBox("Select one Kernel.", 'Info')
                return
            
            kernel = kernels[0]
            
        except Exception as x:
            wx.MessageBox(x.message, 'Info')
            return
        
        wx.CallAfter(Publisher().sendMessage, "CREATE AMI", (self.img, self.region, kernel))
        
        self._view.EndModal(wx.ID_OK)
    
    def setImage(self, img):
        self.img = img
    
                
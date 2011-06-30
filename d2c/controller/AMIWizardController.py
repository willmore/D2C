import wx
from wx.lib.pubsub import Publisher
from .util import createEmptyChecker

class AMIWizardController:
    
    def __init__(self, view, dao):
        
        self.cloud = None
        self.kernel = None
        self.ramdisk = None
        
        self._view = view
        self._dao = dao
         
        self._view.container.getPanel("CLOUD").setClouds(dao.getClouds())
        self._view.container.getPanel("CLOUD").chooseButton.Bind(wx.EVT_BUTTON, self.selectCloud)
        self._view.container.getPanel("CLOUD").cancelButton.Bind(wx.EVT_BUTTON, lambda _: self._view.EndModal(wx.ID_OK))
        
        self._view.container.getPanel("CLOUD").chooseButton.Disable()
        
        self._view.container.getPanel("KERNEL").chooseButton.Bind(wx.EVT_BUTTON, self.selectKernel)
        self._view.container.getPanel("KERNEL").backButton.Bind(wx.EVT_BUTTON, self.showCloud)
        
        createEmptyChecker(self._view.container.getPanel("KERNEL").chooseButton,
                           self._view.container.getPanel("KERNEL").kernelList)
        
        self._view.container.getPanel("RAMDISK").chooseButton.Bind(wx.EVT_BUTTON, self.selectRamdisk)
        self._view.container.getPanel("RAMDISK").backButton.Bind(wx.EVT_BUTTON, self.showKernel)
        
        createEmptyChecker(self._view.container.getPanel("RAMDISK").chooseButton,
                           self._view.container.getPanel("RAMDISK").rdList)
        
        self._view.container.getPanel("BUCKET").createButton.Bind(wx.EVT_BUTTON, self.createAMI)
        self._view.container.getPanel("CLOUD").cloudList.Bind(wx.EVT_LIST_ITEM_SELECTED, self.testContinue)
        self._view.showPanel("CLOUD") 
        
    def testContinue(self, _):
        if self._view.container.getPanel("CLOUD").cloudList.GetSelectedItemCount() == 1:
            self._view.container.getPanel("CLOUD").chooseButton.Enable()
        else:
            self._view.container.getPanel("CLOUD").chooseButton.Disable()
      
    def showPanel(self, label):
        return lambda _: self._view.showPanel(label)
    
    def showCloud(self,_):
        self._view.container.showPanel("CLOUD")
        
    def showKernel(self,_):
        self._view.container.showPanel("KERNEL")
            
    def selectCloud(self, _):
        clouds = self._view.container.getPanel("CLOUD").cloudList.getSelectedItems()
        
        if len(clouds) != 1:
            wx.MessageBox("Please select one Cloud", 'Info')
            return
            
        self.cloud = clouds[0]
        self._view.container.getPanel("KERNEL").kernelList.setItems(self.cloud.kernels)
        self._view.container.getPanel("RAMDISK").rdList.setItems(self.cloud.ramdisks)
        self._view.showPanel("KERNEL")
    
    def selectKernel(self, _):
        
        kernels = self._view.container.getPanel("KERNEL").kernelList.getSelectedItems()
        if len(kernels) != 1:
            wx.MessageBox("Please select one Kernel", 'Info')
            return
        
        self.kernel = kernels[0]
        
        if self.kernel.isPvGrub:
            self._view.showPanel("BUCKET")
        else:
            self._view.showPanel("RAMDISK")
            
    def selectRamdisk(self, _):
        
        rds = self._view.container.getPanel("RAMDISK").rdList.getSelectedItems()
        if len(rds) != 1:
            wx.MessageBox("Please select one Ramdisk", 'Info')
            return
        
        self.ramdisk = rds[0]

        self._view.showPanel("BUCKET")
    
    def createAMI(self, _):
        
        bucket = self._view.container.getPanel("BUCKET").bucket.GetValue()
        
        wx.CallAfter(Publisher().sendMessage, "CREATE AMI", (self.img, self.cloud, self.kernel, bucket, self.ramdisk))
        
        self._view.EndModal(wx.ID_OK)
    
    def setImage(self, img):
        self.img = img
    
                
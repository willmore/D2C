import wx
import os

from .ContainerPanel import ContainerPanel
import d2c.controller.util as util
from d2c.model.SourceImage import Image, DesktopImage, AMI
from .NewAMIWizard import NewAMIWizard
from d2c.controller.AMIWizardController import AMIWizardController
from wx.lib.pubsub import Publisher

class ImageTab(wx.Panel):
    
    def __init__(self, dao, *args, **kwargs):

        wx.Panel.__init__(self, *args, **kwargs)
        
        self.dao = dao
        
        self.splitter = wx.SplitterWindow(self, -1)
        self.splitter.SetMinimumPaneSize(150)
        vbox = wx.BoxSizer(wx.VERTICAL)

        self.tree = wx.TreeCtrl(self.splitter, -1, wx.DefaultPosition, 
                                (-1,-1), wx.TR_HIDE_ROOT|wx.TR_HAS_BUTTONS)
        self.treeRoot = self.tree.AddRoot('root')
        vbox.Add(self.splitter, 1, wx.EXPAND)
        
        self.displayPanel = ContainerPanel(self.splitter, -1)
        
        self.splitter.SplitVertically(self.tree, self.displayPanel)
        
        self.bottomPanel = wx.Panel(self, -1)
        self.bottomPanel.vbox = wx.BoxSizer(wx.VERTICAL)
        self.bottomPanel.addButton = wx.Button(self.bottomPanel, -1, 'Add Image')
        self.bottomPanel.vbox.Add(self.bottomPanel.addButton, 0, wx.ALL, 5)
        self.bottomPanel.SetSizer(self.bottomPanel.vbox)
        self.bottomPanel.SetBackgroundColour('GREY')
        
        vbox.Add(self.bottomPanel, 0, wx.EXPAND)
        
        self.SetSizer(vbox)
        self.Layout()   
        
        self.loadImages()
        
        self.tree.Bind(wx.EVT_TREE_SEL_CHANGED, self.imageSelect)
        self.bottomPanel.addButton.Bind(wx.EVT_BUTTON, self.addImage)
        
        Publisher.subscribe(self.handleNewAMI, "AMI JOB DONE")

    def handleNewAMI(self, msg):
        (_, ami, _, _) = msg.data
        self.addRealPanel(AMIImagePanel(ami, self.displayPanel, -1))
    
    def addRealPanel(self, panel):
        imageName = panel.image.image.name
        item, cookie = self.tree.GetFirstChild(self.treeRoot)
        while item:
            if imageName == self.tree.GetItemText(item):
                self.tree.AppendItem(item, panel.image.amiId)
                self.displayPanel.addPanel(panel.image.amiId, panel)
                return
            item, cookie = self.tree.GetNextChild(self.treeRoot, cookie)
            
        raise Exception("Unable to find node for: %s" % imageName)
    
    def imageSelect(self, event):
        label = self.tree.GetItemText(event.GetItem())
        if label != "root":
            self.displayPanel.showPanel(label)
        
    def loadImages(self):
        self.tree.DeleteChildren(self.treeRoot)
        self.displayPanel.clearPanels()
        for image in self.dao.getImages():
            self.addImagePanel(ImagePanel(image, self.displayPanel, -1))
        
    def addImagePanel(self, imagePanel):         
        node = self.tree.AppendItem(self.treeRoot, imagePanel.image.name)
        self.displayPanel.addPanel(imagePanel.image.name, imagePanel)
        
        for real in imagePanel.image.reals:
            if isinstance(real, AMI):
                self.tree.AppendItem(node, real.amiId) 
                self.displayPanel.addPanel(real.amiId, AMIImagePanel(real, self.displayPanel, -1))
            else:
                self.tree.AppendItem(node, real.path)
                self.displayPanel.addPanel(real.path, DesktopImagePanel(real, self.dao, self.displayPanel, -1))
        
        
    def addImage(self, _):
        dialog = AddImageDialog(self.dao, None, -1, 'Add Image', size=(400,400))
        
        if dialog.ShowModal() == wx.ID_OK:
            deskImg = DesktopImage(None, None, self.dao.getCloud(dialog.cloud.GetValue()), dialog.path.GetValue())
            img = Image(None, dialog.name.GetValue(), deskImg, [deskImg])
            self.dao.add(img)
            self.addImagePanel(ImagePanel(img, self.displayPanel, -1)) 
        
        dialog.Destroy()

    
        
class AddImageDialog(wx.Dialog):
    
    def __init__(self, dao, *args, **kwargs):
        wx.Dialog.__init__(self, *args, **kwargs)
        
        self.name = wx.TextCtrl(self, -1, size=(100, -1))
        self.path = wx.TextCtrl(self, -1, size=(100, -1))
        self.cloud = wx.ComboBox(self, -1, choices=[c.name for c in dao.getClouds()])
        self.browseButton = wx.Button(self, -1, "Browse")
        
        self.browseButton.Bind(wx.EVT_BUTTON, self.onFindImage)
        
        self.createButton = wx.Button(self, -1, "Create")
        self.createButton.Bind(wx.EVT_BUTTON, self.onCreate)
        
        util.createEmptyChecker(self.createButton, self.name, self.path)
        
        self.cancelButton = wx.Button(self, wx.ID_CANCEL, "Cancel")
        
        vbox = wx.BoxSizer(wx.VERTICAL)
        
        vbox.Add(wx.StaticText(self, -1, "Image name:"), 0 , wx.ALL, 2)
        vbox.Add(self.name, 0, wx.EXPAND | wx.ALL, 2)
        
        vbox.Add(wx.StaticText(self, -1, "Add a desktop image:"), 0 , wx.ALL, 2)
        vbox.Add(self.path, 0, wx.EXPAND | wx.ALL, 2)
        vbox.Add(self.browseButton, 0, wx.ALIGN_RIGHT | wx.ALL, 2)
        
        vbox.Add(wx.StaticText(self, -1, "Select associated cloud:"), 0 , wx.ALL, 2)
        vbox.Add(self.cloud, 0, wx.ALL, 2)
        vbox.Add(wx.Panel(self, -1, size=(-1, 10)), 0, wx.EXPAND)
        vbox.Add(self.createButton, 0, wx.ALIGN_RIGHT | wx.ALL, 2)
        vbox.Add(self.cancelButton, 0, wx.ALIGN_RIGHT | wx.ALL, 2)
        
        self.SetSizer(vbox)
    
    def onCreate(self, _):
        self.EndModal(wx.ID_OK)

    def onFindImage(self, _):
        dlg = wx.FileDialog(self, "Choose an image", os.getcwd(), "", "*.*", wx.OPEN)
        if dlg.ShowModal() == wx.ID_OK:
            path = dlg.GetPath()
            self.path.SetValue(path)
        dlg.Destroy()
        
class ImagePanel(wx.Panel):    
    
    def __init__(self, image, *args, **kwargs):
        wx.Panel.__init__(self, *args, **kwargs)
        
        self.SetSizer(wx.BoxSizer(wx.VERTICAL))
        
        self.image = image
        
        label = wx.StaticText(self, -1, image.name)
        label.SetFont(wx.Font(20, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.GetSizer().Add(label, 0, wx.BOTTOM, 10)
        
class AMIImagePanel(wx.Panel):    
    
    def __init__(self, image, *args, **kwargs):
        wx.Panel.__init__(self, *args, **kwargs)
        
        self.SetSizer(wx.BoxSizer(wx.VERTICAL))
        
        self.image = image
        
        label = wx.StaticText(self, -1, image.amiId)
        label.SetFont(wx.Font(20, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.GetSizer().Add(label, 0, wx.BOTTOM, 10)
        
class DesktopImagePanel(wx.Panel):    
    
    def __init__(self, image, dao, *args, **kwargs):
        wx.Panel.__init__(self, *args, **kwargs)
        
        self.dao = dao
        self.sizer = wx.BoxSizer(wx.VERTICAL)
        
        
        self.image = image
        
        label = wx.StaticText(self, -1, image.path)
        label.SetFont(wx.Font(20, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.sizer.Add(label, 0, wx.BOTTOM, 10)
        
        self.createAMIButton = wx.Button(self, -1, "Create AMI")
        self.createAMIButton.Bind(wx.EVT_BUTTON, self.createAMI)
        
        self.sizer.Add(self.createAMIButton)
        self.SetSizer(self.sizer)
        
    def createAMI(self, _):
        amiWiz = NewAMIWizard(None, -1, 'Create AMI', size=(600,300))
        
        controller = AMIWizardController(amiWiz, self.dao)
        controller.setImage(self.image)
        
        amiWiz.ShowModal()
        amiWiz.Destroy()
        
        
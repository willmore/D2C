import wx

from .AMIWizardController import AMIWizardController
from d2c.gui.NewAMIWizard import NewAMIWizard
from d2c.model.SourceImage import SourceImage

from .util import createEmptyChecker  

class ImageController:
    
    def __init__(self, imageView, dao):
        
        self._imageView = imageView
        self._dao = dao
          
        self._imageView.addButton.Bind(wx.EVT_BUTTON, self._onAddImage) 
        self._imageView.SetImages(dao.getSourceImages())
        self._imageView.createAMIButton.Bind(wx.EVT_BUTTON, self._createAMIImage)
        
        createEmptyChecker(self._imageView.addButton, self._imageView.newFileText)
        createEmptyChecker(self._imageView.createAMIButton, self._imageView.list)
        
    def _onAddImage(self, _):
        
        path = self._imageView.newFileText.GetValue()
        
        if path == "":
            wx.MessageBox('Path must not be empty', 'Info')
            return

        self._dao.add(SourceImage(path))
        self._refreshImgList()
        
    def _refreshImgList(self):
        self._imageView.SetImages(self._dao.getSourceImages())
        
    def _createAMIImage(self, _):
        
        img = self._imageView.list.getSelectedItems()[0]
        amiWiz = NewAMIWizard(None, -1, 'Create AMI', size=(600,300))
        
        controller = AMIWizardController(amiWiz, self._dao)
        controller.setImage(img)
        
        amiWiz.ShowModal()
        amiWiz.Destroy()
        
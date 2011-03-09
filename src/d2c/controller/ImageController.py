'''
Created on Feb 16, 2011

@author: willmore
'''
'''
Created on Feb 15, 2011

@author: willmore
'''
import wx
from wx.lib.pubsub import Publisher as pub

from d2c.model.SourceImage import SourceImage


class ImageController:
    
    def __init__(self, imageView, dao):
        
        self._imageView = imageView
        self._dao = dao
          
        self._imageView._addButton.Bind(wx.EVT_BUTTON, self._onAddImage) 
        self._imageView.SetImages(dao.getSourceImages())
        self._imageView.createAMIButton.Bind(wx.EVT_BUTTON, self._createAMIImage)
        
    def _onAddImage(self, event):
        
        path = self._imageView._newFile.GetValue()
        
        if path == "":
            wx.MessageBox('Path must not be empty', 'Info')
            return

        self._dao.addSourceImage(path)
        self._refreshImgList()
        
    def _refreshImgList(self):
        self._imageView.SetImages(self._dao.getSourceImages())
        
    def _createAMIImage(self, event):
        i = self._imageView._list.GetFirstSelected();
        img = self._imageView._list.GetItem(i).GetText()
        pub.sendMessage("CREATE AMI", img)

        
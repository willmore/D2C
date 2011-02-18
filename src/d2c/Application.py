'''
Created on Feb 10, 2011

@author: willmore
'''

import wx
from data.dao import DAO
from gui.Gui import Gui

from controller.ConfController import ConfController
from controller.ImageController import ImageController


class Application:

    def __init__(self):
        
        self._dao = DAO()
        self._app = wx.App()
        
        self._frame = Gui()
        
        self._credController = ConfController(self._frame.getConfigurationPanel(), self._dao)
        self._imageController = ImageController(self._frame.getImagePanel(), self._dao)
        
        self._frame
        self._frame.Show()     
        
    def MainLoop(self):
        self._app.MainLoop()
        

        
        

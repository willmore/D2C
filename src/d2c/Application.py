'''
Created on Feb 10, 2011

@author: willmore
'''

import wx
from d2c.data.DAO import DAO
from d2c.gui.Gui import Gui

from controller.ConfController import ConfController
from controller.ImageController import ImageController
from controller.AMIController import AMIController
from d2c.AMITools import AMIToolsFactory

class Application:

    def __init__(self, amiToolsFactory=AMIToolsFactory()):
        
        self._amiToolsFactory = amiToolsFactory
        self._dao = DAO()
        self._app = wx.App()
        
        self._frame = Gui()
        
        self._credController = ConfController(self._frame.getConfigurationPanel(), self._dao)
        self._imageController = ImageController(self._frame.getImagePanel(), self._dao)
        self._amiController = AMIController(self._frame.getAMIPanel(), self._dao, self._amiToolsFactory)
        
        self._frame
        self._frame.Show()     
        
    def MainLoop(self):
        self._app.MainLoop()
        

        
        

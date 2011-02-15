'''
Created on Feb 10, 2011

@author: willmore
'''

import wx
from data.dao import DAO
from gui.Gui import Gui

from controller.CredController import CredController


class Application:

    def __init__(self):
        
        self._dao = DAO()
        self._app = wx.App()
        
        self._frame = Gui()
        
        self._credController = CredController(self._frame.getCredentialPanel(), self._dao)
        
        self._frame
        self._frame.Show()     
        
    def MainLoop(self):
        self._app.MainLoop()
        

        
        

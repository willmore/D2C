'''
Created on Feb 10, 2011

@author: willmore
'''

import wx
from data.DAO import DAO
from gui.Gui import Gui


class Application:

    def __init__(self):
        
        self._dao = DAO()
        self._app = wx.App()
        
        self._frame = Gui()
        self._frame.Show()
        
        
        
    def MainLoop(self):
        self._app.MainLoop()

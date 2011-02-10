'''
Created on Feb 10, 2011

@author: willmore
'''

import wx
from gui.Gui import Gui


class Application:
    '''
    classdocs
    '''

    def __init__(self):
        '''
        Constructor
        '''
        self.app = wx.App()
        
        self.frame = Gui()
        self.frame.Show()
        
    def MainLoop(self):
        self.app.MainLoop()

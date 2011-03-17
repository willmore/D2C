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
from d2c.gui.DeploymentWizard import DeploymentWizard
from d2c.controller.DeploymentWizardController import DeploymentWizardController


class Application:

    def __init__(self, amiToolsFactory=AMIToolsFactory()):
        
        self._amiToolsFactory = amiToolsFactory
        self._dao = DAO()
        self._app = wx.App()
        
        self._frame = Gui()
        
        self._credController = ConfController(self._frame.getConfigurationPanel(), self._dao)
        self._imageController = ImageController(self._frame.getImagePanel(), self._dao)
        self._amiController = AMIController(self._frame.getAMIPanel(), self._dao, self._amiToolsFactory)
        
        self._frame.bindAddDeploymentTool(self.addDeployment)
        
        self._frame
        self._frame.Show()     
        
        
    def addDeployment(self, event):
        mywiz = DeploymentWizard(None, -1, 'Simple Wizard')
    
        controller = DeploymentWizardController(mywiz, self._dao)
        mywiz.ShowModal()
        mywiz.Destroy()
        
    def MainLoop(self):
        self._app.MainLoop()
        

        
        

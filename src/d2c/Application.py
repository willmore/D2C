'''
Created on Feb 10, 2011

@author: willmore
'''

import wx
from wx.lib.pubsub import Publisher

from d2c.data.DAO import DAO
from d2c.gui.Gui import Gui
from controller.ConfController import ConfController
from controller.ImageController import ImageController
from controller.AMIController import AMIController
from d2c.AMITools import AMIToolsFactory
from d2c.gui.DeploymentWizard import DeploymentWizard
from d2c.controller.DeploymentWizardController import DeploymentWizardController
from .gui.DeploymentPanel import DeploymentPanel
from .controller.DeploymentController import DeploymentController


class Application:

    def __init__(self):
        
        self._amiToolsFactory = AMIToolsFactory()
        self._dao = DAO()
        self._app = wx.App()
        
        self._frame = Gui()
        
        self._credController = ConfController(self._frame.getConfigurationPanel(), self._dao)
        self._imageController = ImageController(self._frame.getImagePanel(), self._dao)
        self._amiController = AMIController(self._frame.getAMIPanel(), self._dao, self._amiToolsFactory)
        self.loadDeploymentPanels()
        
        self._frame.bindAddDeploymentTool(self.addDeployment)
        
        Publisher.subscribe(self._handleNewDeployment, "DEPLOYMENT CREATED")
        
        self._frame
        self._frame.Show()     
        
    def _handleNewDeployment(self, msg):    
        deployment = msg.data['deployment']
        self.loadDeploymentPanel(deployment)
        self._frame.setSelection("[Deployment] " + deployment.id)    
        
    def loadDeploymentPanels(self):
        self.deplomentControllers = {}
        for d in self._dao.getDeployments():
            self.loadDeploymentPanel(d)
            
    def loadDeploymentPanel(self, deployment):
        deployPanel = DeploymentPanel(deployment, self._frame._containerPanel)
        self._frame.addPanel("[Deployment] " + deployment.id, deployPanel)
        self.deplomentControllers[deployment.id] = DeploymentController(deployPanel, self._dao)
    
    def addDeployment(self, event):
        mywiz = DeploymentWizard(None, -1, 'Simple Wizard')
    
        controller = DeploymentWizardController(mywiz, self._dao)
        mywiz.ShowModal()
        mywiz.Destroy()
        
    
        
    def MainLoop(self):
        self._app.MainLoop()
        

        
        

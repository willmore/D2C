'''
Created on Feb 10, 2011

@author: willmore
'''

import wx
from wx.lib.pubsub import Publisher

from d2c.gui.Gui import Gui
from d2c.controller.ConfController import ConfController

from d2c.controller.CompCloudConfController import CompCloudConfController
from d2c.gui.ConfPanel import ConfPanel
from d2c.gui.CompCloudConfPanel import CompCloudWizard
from d2c.controller.ImageController import ImageController
from d2c.controller.StorageWizardController import StorageWizardController
from d2c.controller.AMIController import AMIController
from d2c.gui.DeploymentWizard import DeploymentWizard
from d2c.gui.ImageStoreWizard import ImageStoreWizard
from d2c.controller.DeploymentWizardController import DeploymentWizardController
from d2c.gui.DeploymentPanel import DeploymentPanel
from d2c.controller.DeploymentController import DeploymentController


class Application:

    def __init__(self, dao, amiToolsFactory):
        
        self._amiToolsFactory = amiToolsFactory
        self._dao = dao
        self._app = wx.App()

        self._frame = Gui()
        
        self._imageController = ImageController(self._frame.getImagePanel(), self._dao)
        self._amiController = AMIController(self._frame.getAMIPanel(), self._dao, self._amiToolsFactory)
        self.loadDeploymentPanels()
        
        self._frame.bindAddDeploymentTool(self.addDeployment)
        self._frame.bindConfTool(self.showConf)
        self._frame.bindCompCloudConfTool(self.showCompCloudConf)
        self._frame.bindStoreTool(self.showStoreWizard)
        
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
        mywiz = DeploymentWizard(None, -1, 'Deployment Creation Wizard', size=(600,400))
    
        controller = DeploymentWizardController(mywiz, self._dao)
        mywiz.ShowModal()
        mywiz.Destroy()
        
    def showConf(self, event):
        
        conf = ConfPanel(None, size=(600,300))
        controller = ConfController(conf, self._dao)
        conf.ShowModal()
        conf.Destroy()
        
    def showCompCloudConf(self, event):
        
        cloudWiz = CompCloudWizard(None, -1, 'Computation Clouds', size=(600,300))
        controller = CompCloudConfController(cloudWiz, self._dao)
        cloudWiz.ShowModal()
        cloudWiz.Destroy()
        
    def showStoreWizard(self, event):
        
        storageWiz = ImageStoreWizard(None, -1, 'Storage Clouds', size=(600,300))
        controller = StorageWizardController(storageWiz, self._dao)
        storageWiz.ShowModal()
        storageWiz.Destroy()
        
    def MainLoop(self):
        self._app.MainLoop()
        
        

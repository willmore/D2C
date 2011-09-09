'''
Created on Feb 10, 2011

@author: willmore
'''

import wx
from wx.lib.pubsub import Publisher

from d2c.gui.Gui import Gui
from d2c.controller.ConfController import ConfController

from d2c.gui.ConfPanel import ConfPanel
from d2c.controller.ImageController import ImageController
from d2c.controller.AMIController import AMIController
from d2c.gui.DeploymentTemplateWizard import DeploymentTemplateWizard
from d2c.controller.DeploymentTemplateWizardController import DeploymentTemplateWizardController
from d2c.controller.DeploymentController import DeploymentController, DeploymentTemplateController
from d2c.controller.NewCloudController import NewCloudController
from d2c.gui.CloudPanel import CloudWizard
from d2c.gui.DeploymentTab import DeploymentTemplatePanel



class Application:

    def __init__(self, dao, amiToolsFactory):
        
        self._amiToolsFactory = amiToolsFactory
        self._dao = dao
        self._app = wx.App()

        self._frame = Gui(dao)
    
        self._imageController = ImageController(self._frame.imagePanel, self._dao)
        self._amiController = AMIController(self._frame.amiPanel, 
                                            self._dao,
                                            self._amiToolsFactory)
        
        self.loadDeploymentPanels()
        
        self._frame.bindAddDeploymentTool(self.addDeployment)
        self._frame.bindConfTool(self.showConf)
        self._frame.bindCloudTool(self.showCloudWizard)
        
        self._frame.deploymentPanel.tree.Bind(wx.EVT_TREE_SEL_CHANGED, self.deploymentSelect)
        
        Publisher.subscribe(self._handleNewDeploymentTemplate, "DEPLOYMENT TEMPLATE CREATED")
        Publisher.subscribe(self._handleNewDeployment, "DEPLOYMENT CREATED")
        Publisher.subscribe(self._handleDeleteDeployment, "DELETE DEPLOYMENT")
        
        self._frame
        self._frame.Show()     
            
    def deploymentSelect(self, event):
        self._frame.deploymentPanel.displayPanel.showPanel(self._frame.deploymentPanel.tree.GetItemText(event.GetItem()))
    
    def _handleDeleteDeployment(self, msg):    
        deployment = msg.data['deployment']
        self._frame.deploymentPanel.removeDeployment(deployment)
        self._dao.delete(deployment)
    
    def _handleNewDeploymentTemplate(self, msg):    
        deployment = msg.data['deployment']
        self.loadDeploymentPanel(deployment)  
        
    def _handleNewDeployment(self, msg):    
        deployment = msg.data['deployment']
        self._frame.deploymentPanel.addDeployment(deployment)      
        
    def loadDeploymentPanels(self):
        self.deplomentControllers = {}
        for d in self._dao.getDeploymentTemplates():
            self.loadDeploymentPanel(d)
            
    def loadDeploymentPanel(self, deployment):
        
        deployPanel = DeploymentTemplatePanel(deployment, self._dao, self._frame.deploymentPanel.displayPanel)
        self._frame.deploymentPanel.addDeploymentTemplatePanel(deployPanel)
        self.deplomentControllers[deployment.id] = DeploymentTemplateController(deployPanel, self._dao)
    
    def addDeployment(self, event):
        mywiz = DeploymentTemplateWizard(None, -1, 'Deployment Template Creation Wizard')
        DeploymentTemplateWizardController(mywiz, self._dao)
        mywiz.ShowModal()
        mywiz.Destroy()
        
    def showConf(self, event):
        
        conf = ConfPanel(None, size=(800,400))
        ConfController(conf, self._dao)
        conf.ShowModal()
        conf.Destroy()
        
    def showCloudWizard(self, event):
        
        cloudWiz = CloudWizard(None, -1, 'Manage Clouds', size=(500,400))
        NewCloudController(cloudWiz, self._dao)
        cloudWiz.ShowModal()
        cloudWiz.Destroy()
        
    def MainLoop(self):
        self._app.MainLoop()
        
        

'''
Created on Feb 27, 2011

@author: willmore
'''
import wx
from wx.lib.pubsub import Publisher
import traceback
from threading import Thread
from d2c.AMICreator import AMICreator
from d2c.AMITools import AMIToolsFactory


class _AmiLogMsg:
        
    def __init__(self,img,msg):
        self.img = img
        self.msg = msg


class AMIController:
    
    JOB_CODE_SUCCESS = 0
    
    def __init__(self, amiView, dao, amiToolsFactory=AMIToolsFactory()):
        self.__dao = dao
        self.__amiView = amiView
        self.__amiToolsFactory = amiToolsFactory
        
        Publisher.subscribe(self.__createAMI, "CREATE AMI")
        Publisher.subscribe(self._handleAMILog, "AMI LOG")
        Publisher.subscribe(self._handleAMIJobDone, "AMI JOB DONE")
    
    def _handleAMIJobDone(self, msg):
        (jobId, amiId, code, execption) = msg
        
        if code != self.JOB_CODE_SUCCESS:
            self.__dao.upateAMIJob()
    
    def _handleAMILog(self, msg):
        
        self.__amiView.appendLogPanelText(msg.data.img, msg.data.msg)
        
    def _sendFinishMessage(self, jobid, amiid=None, code=JOB_CODE_SUCCESS, exception=None):
        wx.CallAfter(Publisher().sendMessage, "AMI JOB DONE", (jobid, amiid, code, exception))
        
    class __AMIThread(Thread):
            
            def __init__(self, img, conf, amiToolsFactory, logger):
                Thread.__init__(self)
                self.__img = img
                self.__amiToolsFactory = amiToolsFactory
                self.__conf = conf
                self.__logger = logger
                
            def run(self):
                try:
                    amiid = AMICreator(self.__img, 
                                       self.__conf.ec2Cred, 
                                       self.__conf.awsUserId, 
                                       "et.cs.ut.cloud",
                                       amiTools=self.__amiToolsFactory.getAMITools(
                                                         self.__conf.ec2ToolHome, 
                                                         self.__conf.awsCred.access_key_id, 
                                                         self.__conf.awsCred.secret_access_key,
                                                         self.__logger),
                                       logger=self.__logger
                                       ).createAMI()
                                       
                    self._sendFinishMessage(self, amiid, code=JOB_CODE_SUCCESS, exception=None)
                                   
                except:
                    traceback.print_exc()
                    
        
    def __createAMI(self, msg):
        
        rawImg = msg.data.encode('ascii')
        
        self.__amiView.addLogPanel(rawImg)
        self.__amiView.showLogPanel(rawImg)


        amiThread = self.__AMIThread(rawImg, 
                                     self.__dao.getConfiguration(),
                                     self.__amiToolsFactory,
                                     self.__createLogger(rawImg))
        amiThread.start()  
     
    class __CreationLogger:
        
        def __init__(self, img):
            self._img = img
        
        def write(self, msg):
            wx.CallAfter(Publisher().sendMessage, "AMI LOG", _AmiLogMsg(self._img, msg))
    
    def __createLogger(self, img):
        return self.__CreationLogger(img)
    
        
                
        
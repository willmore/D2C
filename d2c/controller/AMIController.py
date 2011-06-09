'''
Created on Feb 27, 2011

@author: willmore
'''
import wx
from wx.lib.pubsub import Publisher
import traceback
from threading import Thread
from d2c.AMICreator import AMICreator

class _AmiLogMsg:
        
    def __init__(self, img, msg):
        self.img = img
        self.msg = msg
        
class Codes:
    JOB_CODE_SUCCESS=0

class AMIThread(Thread):
            
    def __init__(self, img, conf, amiToolsFactory, s3Bucket, 
                 s3Storage, region, dao, logger):
        
        Thread.__init__(self)
        self.__img = img
        self.__amiToolsFactory = amiToolsFactory
        self.__conf = conf
        self.__s3Bucket = s3Bucket
        self.__region = region
        self.__s3Storage = s3Storage
        self.__dao = dao
        self.__logger = logger
            
    def _sendFinishMessage(self, jobid, amiid=None, 
                            code=Codes.JOB_CODE_SUCCESS, exception=None):
    
        wx.CallAfter(Publisher().sendMessage, "AMI JOB DONE", 
                             (jobid, amiid, code, exception))
            
    def run(self):
        try:
            ami =AMICreator(self.__img, 
                 self.__conf.ec2Cred, 
                 awsCred,
                 self.__conf.awsUserId, 
                 self.__s3Bucket,
                 self.__region, 
                 self.__s3Storage,
                 self.__dao,
                 logger=self.__logger)          

            self._sendFinishMessage(self.__img, ami, code=Codes.JOB_CODE_SUCCESS, exception=None)
                                   
        except:
            traceback.print_exc()

class AMIController:
    
    
    def __init__(self, amiView, dao, amiToolsFactory):
        self.__dao = dao
        self.__amiView = amiView
        self.__amiToolsFactory = amiToolsFactory
    
        self.__refreshAMIList()
        
        Publisher.subscribe(self.__createAMI, "CREATE AMI")
        Publisher.subscribe(self._handleAMILog, "AMI LOG")
        Publisher.subscribe(self._handleAMIJobDone, "AMI JOB DONE")
    
    def __refreshAMIList(self):
        self.__amiView.setAMIs(self.__dao.getAMIs())
    
    def _handleAMIJobDone(self, msg):
        (_, ami, _, _) = msg.data
        self.__amiView.addAMIEntry(ami)   
    
    def _handleAMILog(self, msg):
        
        self.__amiView.appendLogPanelText(msg.data.img, msg.data.msg)              
        
    def __createAMI(self, msg):
        '''
        Listens for AMI creation request, which are generally sent from ImageController.
        1. Create and display a new log panel which display AMI creation log.
        2. Spawn an AMIThread which creates the AMI.
        3. Add a new entry into the AMI list with the in-creation-progress AMI information.
        '''
        
        rawImg = msg.data.encode('ascii')
        
        self.__amiView.addLogPanel(rawImg)
        self.__amiView.showLogPanel(rawImg)

        amiThread = AMIThread(rawImg, 
                                     self.__dao.getConfiguration(),
                                     self.__amiToolsFactory,
                                     self.__createLogger(rawImg),
                                     self.__dao)
        amiThread.start()
        
        self.__amiView.addAMIEntry(name=rawImg)
     
    class __CreationLogger:
        
        def __init__(self, img):
            self._img = img
        
        def write(self, msg):
            wx.CallAfter(Publisher().sendMessage, "AMI LOG", _AmiLogMsg(self._img, msg))
    
    def __createLogger(self, img):
        return self.__CreationLogger(img)
        
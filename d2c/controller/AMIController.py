'''
Created on Feb 27, 2011

@author: willmore
'''
import wx
from wx.lib.pubsub import Publisher
import traceback
from threading import Thread
from d2c.AMICreator import AMICreator
import time
        
class Codes:
    JOB_CODE_SUCCESS=0

class AMIThread(Thread):
            
    def __init__(self, img, conf, amiToolsFactory, 
                  cloud, kernel, s3Bucket, dao, logger):
        
        Thread.__init__(self)
        self.__img = img
        self.__amiToolsFactory = amiToolsFactory
        self.__conf = conf
        self.__s3Bucket = s3Bucket
        self.__cloud = cloud
        self.__kernel = kernel
        self.__dao = dao
        self.__logger = logger
            
    def _sendFinishMessage(self, jobid, amiid=None, 
                            code=Codes.JOB_CODE_SUCCESS, exception=None):
    
        wx.CallAfter(Publisher().sendMessage, "AMI JOB DONE", 
                             (jobid, amiid, code, exception))
            
    def run(self):
        try:
            amiCreator = AMICreator(self.__img, 
                 self.__conf.ec2Cred, 
                 self.__conf.awsCred,
                 self.__conf.awsUserId, 
                 self.__s3Bucket,
                 self.__cloud,
                 self.__kernel,
                 self.__dao,
                 self.__amiToolsFactory,
                 logger=self.__logger)  
            
            ami = amiCreator.createAMI()
            
            self._sendFinishMessage(self.__img, ami, code=Codes.JOB_CODE_SUCCESS, exception=None)
                                   
        except:
            traceback.print_exc()


class AMIController:
    
    def __init__(self, amiView, dao, amiToolsFactory):
        self.__dao = dao
        self.__amiView = amiView
        self.__amiToolsFactory = amiToolsFactory
    
        self.__refreshAMIList()
        self.__amiView._list.Bind(wx.EVT_LIST_ITEM_SELECTED, self.showAMI)
        Publisher.subscribe(self.__createAMI, "CREATE AMI")
        Publisher.subscribe(self._handleAMIJobDone, "AMI JOB DONE")
    
    def showAMI(self, _):
        
        if self.__amiView._list.GetSelectedItemCount() == 1:
            i = self.__amiView._list.GetFirstSelected()
            print i
            #self.__amiView.showLogPanel()
    
    def __refreshAMIList(self):
        self.__amiView.setAMIs(self.__dao.getAMIs())
    
    def _handleAMIJobDone(self, msg):
        (_, ami, _, _) = msg.data
        #self.__amiView.addAMIEntry(ami)   
        print "TODO handle done"
    
    def __createAMI(self, msg):
        '''
        Listens for AMI creation request, which are currently sent from ImageController.
        1. Create and display a new log panel which display AMI creation log.
        2. Spawn an AMIThread which creates the AMI.
        3. Add a new entry into the AMI list with the in-creation-progress AMI information.
        '''
        
        rawImg,cloud,kernel,s3Bucket = msg.data

        #Create a logger that will capture process output and display in a GUI panel        
        logger = self.__createLogger(rawImg)
        self.__amiView.addLogPanel(logger._channelId)
        self.__amiView.showLogPanel(logger._channelId)
        self.__amiView.Refresh()

        amiThread = AMIThread(rawImg, 
                              self.__dao.getConfiguration(),
                              self.__amiToolsFactory,
                              cloud,
                              kernel,
                              s3Bucket,
                              self.__dao,
                              logger)
        
        amiThread.start()
        
        self.__amiView._list.Append(("---", rawImg,'In Progress', ''))
     
    class __CreationLogger:
        
        def __init__(self, img):
            self._img = img
            self._channelId = "AMI_CREATION_LOG_%d" % time.time()
            
        def write(self, msg):
            wx.CallAfter(Publisher().sendMessage, self._channelId, msg)
    
    
    def receiveLogMessage(self, msg):
        self.__amiView.appendLogPanelText(msg.topic[0], msg.data)
            
    def __createLogger(self, img):
        logger = self.__CreationLogger(img)
        
        Publisher.subscribe(self.receiveLogMessage, 
                            logger._channelId)
        return logger
        
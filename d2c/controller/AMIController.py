
import wx
from wx.lib.pubsub import Publisher
import traceback
from threading import Thread
from d2c.AMICreator import AMICreator
import time

     
class Codes:
    JOB_CODE_SUCCESS=0

class AMIThread(Thread):
    '''
    Thread that encapsulates creation of a new AMI from a source VM image.
    '''
             
    def __init__(self, img, cloudCred, amiToolsFactory, 
                  cloud, kernel, s3Bucket, dao, logger, ramdisk):
        
        Thread.__init__(self)
        self.__img = img
        self.__amiToolsFactory = amiToolsFactory
        self.__cloudCred = cloudCred
        self.__s3Bucket = s3Bucket
        self.__cloud = cloud
        self.__kernel = kernel
        self.__dao = dao
        self.__logger = logger
        self.__ramdisk = ramdisk
            
    def _sendFinishMessage(self, jobid, ami=None, 
                            code=Codes.JOB_CODE_SUCCESS, exception=None):
    
        wx.CallAfter(Publisher().sendMessage, "AMI JOB DONE", 
                             (jobid, ami, code, exception))
            
    def run(self):
        try:
            amiCreator = AMICreator(self.__img, 
                 self.__cloudCred.ec2Cred, 
                 self.__cloudCred.awsCred,
                 self.__cloudCred.awsUserId, 
                 self.__s3Bucket,
                 self.__cloud,
                 self.__kernel,
                 self.__dao,
                 self.__amiToolsFactory,
                 logger=self.__logger,
                 ramdisk=self.__ramdisk)  
            
            ami = amiCreator.createAMI()
            
            self._sendFinishMessage(self.__img, ami, code=Codes.JOB_CODE_SUCCESS, exception=None)
                                   
        except:
            traceback.print_exc()

class AMITracker(object):

    def __init__(self, ami=None, srcImg=None, cloud=None):
        if ami is not None:
            self.ami = ami
            self.id = ami.amiId
            self.status = "Created"
            self.srcImg = ami.image.originalImage
            self.cloud = ami.cloud
        else:
            self.id = "---"
            self.srcImg = srcImg
            self.status = "In Progress"    
            self.cloud = cloud
        

class AMIController(object):
    
    def __init__(self, amiView, dao, amiToolsFactory):
        self.__dao = dao
        self.__amiView = amiView
        self.__amiToolsFactory = amiToolsFactory

        self.__refreshAMIList()
        self.__amiView.list.Bind(wx.EVT_LIST_ITEM_SELECTED, self.showAMI)
        Publisher.subscribe(self.__createAMI, "CREATE AMI")
        Publisher.subscribe(self._handleAMIJobDone, "AMI JOB DONE")
    
    def showAMI(self, _):
        
        if self.__amiView.list.GetSelectedItemCount() == 1:
            i = self.__amiView.list.GetFirstSelected()
            print i
            #self.__amiView.showLogPanel()
    
    def __refreshAMIList(self):
        
        self.__amiView.setAMIs([AMITracker(a) for a in self.__dao.getAMIs()])
    
    def _handleAMIJobDone(self, msg):
        (_, amiTracker, _, _) = msg.data
        self.__refreshAMIList()
    
    def __createAMI(self, msg):
        '''
        Listens for AMI creation request, which are currently sent from ImageController.
        1. Create and display a new log panel which display AMI creation log.
        2. Spawn an AMIThread which creates the AMI.
        3. Add a new entry into the AMI list with the in-creation-progress AMI information.
        '''
        
        rawImg,cloud,kernel,s3Bucket,ramdisk,cloudCred = msg.data

        #Create a logger that will capture process output and display in a GUI panel        
        logger = self.__createLogger(rawImg.path)
        self.__amiView.addLogPanel(logger._channelId)
        self.__amiView.showLogPanel(logger._channelId)
        self.__amiView.Refresh()

        amiThread = AMIThread(rawImg,
                              cloudCred,
                              self.__amiToolsFactory,
                              cloud,
                              kernel,
                              s3Bucket,
                              self.__dao,
                              logger,
                              ramdisk)
        
        amiThread.amiTracker = AMITracker(srcImg=rawImg, cloud=cloud)
        amiThread.start()
        
        self.__amiView.list.addItem(amiThread.amiTracker)
        
     
     
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
        
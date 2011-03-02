'''
Created on Feb 27, 2011

@author: willmore
'''
import wx
from wx.lib.pubsub import Publisher
import traceback
from threading import Thread
from d2c.AMICreator import AMICreator
from d2c.AMICreator import AMITools

class _AmiLogMsg:
        
        def __init__(self,img,msg):
            self.img = img
            self.msg = msg


class AMIController:
    
    def __init__(self, amiView, dao):
        self.__dao = dao
        self.__amiView = amiView
        
        Publisher.subscribe(self.__createAMI, "CREATE AMI")
        Publisher.subscribe(self._handleAMILog, "AMI LOG")
    
    def _handleAMILog(self, msg):
        
        self.__amiView.appendLogPanelText(msg.data.img, msg.data.msg)
        
    class __AMIThread(Thread):
            
            def __init__(self, img, conf, logger):
                Thread.__init__(self)
                self.__img = img
                self.__conf = conf
                self.__logger = logger
                
            def run(self):
                try:
                    amiid = AMICreator(self.__img, 
                                       self.__conf.ec2Cred, 
                                       self.__conf.awsUserId, 
                                       "et.cs.ut.cloud",
                                       amiTools=AMITools(self.__conf.ec2ToolHome, 
                                                         self.__conf.awsCred.access_key_id, 
                                                         self.__conf.awsCred.secret_access_key),
                                       logger=self.__logger
                                       ).createAMI()
                                   
                except:
                    traceback.print_exc()
                    
        
    def __createAMI(self, msg):
        
        rawImg = msg.data.encode('ascii')
        
        self.__amiView.addLogPanel(rawImg)
        self.__amiView.showLogPanel(rawImg)

        amiThread = self.__AMIThread(rawImg, 
                                     self.__dao.getConfiguration(), 
                                     self.__createLogger(rawImg))
        amiThread.start()  
        #self.__amiView.Layout()
     
    class __CreationLogger:
        
        def __init__(self, img):
            self._img = img
        
        def write(self, msg):
            wx.CallAfter(Publisher().sendMessage, "AMI LOG", _AmiLogMsg(self._img, msg))
    
    def __createLogger(self, img):
        return self.__CreationLogger(img)
    
        
                
        
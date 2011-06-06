
import boto.ec2
from d2c.data.CredStore import CredStore
from d2c.logger import StdOutLogger

class EC2ConnectionFactory:
    
    
    def __init__(self, *args):
        
        self.__ec2Conn = None
        
        if isinstance(args[0], CredStore):
            self.newConstructor(args[0], StdOutLogger() if len(args) == 1 else args[1])
        elif isinstance(args[0], basestring):
            self.oldConstructor(args)
        else:
            raise Exception("Invalid arg type: %s" % type(args[0]))
    
    def newConstructor(self, credStore, logger):    
        self.credStore = credStore
        self.__logger = logger
        
    def oldConstructor(self, args):
        self.__logger = args[2]
        self.__accessKey = args[0]
        self.__secretKey = args[1]
        self.__ec2Conn = None
        
    def getConnection(self, region):
        
        #TODO worry about thread safety?
        #TODO un-hardcode
        #region = "eu-west-1" if region is None else region
        
        assert region is not None
        
        if hasattr(self, 'credStore'):
            cred = self.credStore.getDefaultAWSCred()
            aws_access_key_id =  cred.access_key_id
            aws_secret_access_key = cred.secret_access_key
        else:
            aws_access_key_id =  self.__accessKey
            aws_secret_access_key = self.__secretKey
         
        if self.__ec2Conn is None:
            #TODO: add timeout - if network connection fails, this will spin forever
            self.__logger.write("Initiating connection to ec2 region '%s'..." % region)
            self.__ec2Conn = boto.ec2.connect_to_region(region, 
                                                        aws_access_key_id=aws_access_key_id, 
                                                        aws_secret_access_key=aws_secret_access_key)
            self.__logger.write("EC2 connection established")
            
        return self.__ec2Conn
    
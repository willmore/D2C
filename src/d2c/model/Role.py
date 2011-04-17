   
from d2c.logger import StdOutLogger   
from d2c.model.InstanceType import InstanceType

import time   

class Role:
    
    def __init__(self, deploymentId, 
                 name, ami, count,
                 instanceType, 
                 reservationId=None,
                 startActions=(), 
                 finishedChecks=(),
                 dataCollectors=(), 
                 ec2ConnFactory=None,
                 credStore=None,
                 pollRate=15,
                 logger=StdOutLogger()):
        
        assert count > 0, "Count must be int > 0"
        assert isinstance(instanceType, InstanceType)
        
        self.deploymentId = deploymentId
        self.name = name
        self.ami = ami  
        self.count = count
        self.instanceType = instanceType
        self.pollRate = pollRate
        self.reservationId = reservationId
        self.reservation = None #lazy loaded
        
        self.startActions = list(startActions)
        self.finishedChecks = list(finishedChecks)
        self.dataCollectors = list(dataCollectors)
        
        self.ec2ConnFactory = ec2ConnFactory
        self.credStore = credStore
          
        self.logger = logger
      
    def getReservationId(self):  
        return self.reservationId
    
    def setReservationId(self, id):
        self.reservationId = id
    
    def getName(self):
        return self.name
    
    def getCount(self):
        return self.count
    
    def setEC2ConnFactory(self, ec2ConnFactory):
        self.ec2ConnFactory = ec2ConnFactory  
        
    def launch(self):
        
        self.logger.write("Reserving %d instance(s) of %s" % (self.count, self.ami.amiId))
       
        ec2Conn = self.ec2ConnFactory.getConnection()
        keyName = self.credStore.getDefaultEC2Cred().id
       
        #TODO catch exceptions
        self.reservation = ec2Conn.run_instances(self.ami.amiId, 
                                                 key_name = keyName,
                                                 min_count=self.count, 
                                                 max_count=self.count, 
                                                 instance_type=self.instanceType.name) 
        
        #TODO introduce abstraction appropriate exception
        assert self.reservation is not None and self.reservation.id is not None
        
        self.reservationId = self.reservation.id
        
        self.logger.write("Instance(s) reserved")    
        
    def executeStartCommands(self):
        '''
        Execute the start action(s) on each instance within the role.
        '''
        
        if self.reservation is None:
            self.reservation = self.__getReservation()
        
        for instance in self.reservation.instances: 
            
            instance.update()
            
            for action in self.startActions:
                action.credStore = self.credStore
                action.execute(instance)
                
    def checkFinished(self):
        '''
        Connect to remote instances associated with this role. If each instance satisfies the done condition,
        return True, else return False. 
        '''
        
        if self.reservation is None:
            self.reservation = self.__getReservation()
            
        for instance in self.reservation.instances:
            for check in self.finishedChecks:
                if not check.check(instance):
                    self.logger.write("Returning False for finished test")
                    return False
                
        self.logger.write("Returning true for finished test")        
        
        return True
        
    def __getReservation(self):
        '''
        Update the reservation field with current information from AWS.
        '''    
        
        assert hasattr(self, 'reservationId') and self.reservationId is not None
        
        reservations = self.ec2ConnFactory.getConnection().get_all_instances(filters={'reservation-id':[self.reservationId]})
        
        assert len(reservations) == 1, "Unable to retrieve reservation %s" % self.reservationId
        return reservations[0] 
        
    def collectData(self):
        
        if self.reservation is None:
            self.reservation = self.__getReservation()
        
        for collector in self.dataCollectors:
            for instance in self.reservation.instances:
                collector.collect(instance)
    
    def shutdown(self):
        
        if self.reservation is None:
            self.reservation = self.__getReservation()
          
        #Request the instances be terminated  
        for instance in self.reservation.instances:
            instance.terminate()
        
        #Monitor until all are terminated
        monitorInstances = self.reservation.instances
       
        while len(monitorInstances) > 0:
            self.logger.write("Shutdown monitor length = %d" % len(monitorInstances))
            for instance in self.reservation.instances:
                instance.update()
                self.logger.write("Instance state = %s" % instance.state)
                  
            monitorInstances = filter(lambda inst: inst.state != 'terminated', monitorInstances) 
            
            for instance in self.reservation.instances:
                instance.terminate()
                
            time.sleep(self.pollRate)
        
        self.logger.write("Reservation %s terminated" % self.reservation.id)
    def __str__(self):
        return "{name:%s, ami: %s, instances: %s}" % (self.name, self.ami, str(self.instances))

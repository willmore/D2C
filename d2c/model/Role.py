from d2c.logger import StdOutLogger   
from d2c.model.InstanceType import InstanceType
from d2c.model.Action import Action
from d2c.model.Deployment import Deployment
from d2c.model.AWSCred import AWSCred
import string
import os
from copy import copy

import time   

class Role(object):
    
    def __init__(self,  
                 id, 
                 image, 
                 count,
                 instanceType, 
                 template,
                 deployment=None,
                 reservationId=None,
                 remoteExecutorFactory=None,
                 startActions=(),
                 asyncStartActions=(), 
                 uploadActions=(),
                 finishedChecks=(), 
                 dataCollectors=(),
                 pollRate=15,
                 logger=StdOutLogger()):
        
        assert count > 0, "Count must be int > 0"
        assert instanceType is None or isinstance(instanceType, InstanceType), "Type is %s" % type(instanceType)
        assert deployment is None or isinstance(deployment, Deployment)
        
        self.id = id
        self.image = image  
        self.template = template
        self.count = count
        self.instanceType = instanceType
        self.pollRate = pollRate
        self.reservationId = reservationId
        self.reservation = None #lazy loaded
        self.deployment = deployment
        self.remoteExecutorFactory = remoteExecutorFactory
        self.startActions= list(startActions)
        self.asyncStartActions = list(asyncStartActions)
        self.uploadActions= list(uploadActions)
        self.finishedChecks= list(finishedChecks)
        self.dataCollectors= list(dataCollectors)
        self.logger = logger
        self.sshCred = None
        
    def clone(self):
        '''
        Make a clone of this Role. Id will be set to None.
        Actions will be cloned as well.
        '''
        c = Role(None, image=self.image, count=self.count, instanceType=self.instanceType, 
                    template=self.template)
        
        c.startActions = [a.copy() for a in self.startActions]
        for a in c.startActions:
            a.id = None
            
        c.asyncStartActions = [a.copy() for a in self.asyncStartActions]
        for a in c.asyncStartActions:
            a.id = None
            
        c.uploadActions = [a.copy() for a in self.uploadActions]
        for a in c.uploadActions:
            a.id = None
            
        c.finishedChecks = [a.copy() for a in self.finishedChecks]
        for a in c.finishedChecks:
            a.id = None
            
        c.dataCollectors = [a.copy() for a in self.dataCollectors]
        for a in c.dataCollectors:
            a.id = None
        
        return c
    
    def getName(self):
        return self.template.name
    
    def getDataDirectory(self):
        return os.path.join(self.deployment.dataDir, str(self.id))
    
    def setSSHCred(self, sshCred):
        self.sshCred = sshCred
        
        for collection in [self.startActions, self.asyncStartActions, self.uploadActions, self.finishedChecks, self.dataCollectors]:
            for a in collection:
                a.sshCred = sshCred
    
    def setLogger(self, logger): 
    
        self.logger = logger
        
        self._cascadeLogger()
        
    def _cascadeLogger(self):
        
        for actions in [self.startActions, self.uploadActions, 
                        self.finishedChecks, 
                        self.dataCollectors]:
            for a in actions:
                a.logger = self.logger
      
    def getReservationId(self):  
        return self.reservationId
    
    def setReservationId(self, id):
        self.reservationId = id
    
    def getCount(self):
        return self.count
     
    def costPerHour(self):
        return self.count * self.instanceType.costPerHour if self.instanceType is not None else 0
       
    def launch(self, awsCred):
        
        assert awsCred is None or isinstance(awsCred, AWSCred)
        
        cloudConn = self.deployment.cloud.getConnection(awsCred)
       
        self.logger.write("Reserving %d instance(s) of %s with launchKey %s" % (self.count, str(self.image.image.name), self.sshCred.name))
       
        #TODO catch exceptions     
        self.reservation = cloudConn.runInstances(self.image, 
                                                 count=self.count, 
                                                 instanceType=self.instanceType,
                                                 keyName=self.sshCred.name) 
        
        #TODO introduce abstraction-appropriate exception
        assert self.reservation is not None and self.reservation.id is not None
        
        self.reservationId = self.reservation.id
        
        self.logger.write("Instance(s) reserved with ID: %s" % self.reservationId)    
     
    def __executeActions(self, actions, shellVars=None): 
        '''
        Execute the actions on each instance within the role.
        '''
        if self.reservation is None:
            self.reservation = self.__getReservation()
        
        for instance in self.reservation.instances: 
            instance.update()
            
            for action in actions:
                action.execute(instance)
                
    def setIPContext(self, ips):
        '''
        In the current implementation, this method will create file 
        /tmp/d2c.context
        on all hosts with a '\n' delimited line of private IPs.
        This will be reworked in the the future to create a more full-featured 
        context scheme.
        '''
        
        ctxt = string.join(ips, "\n")
        cmd = "echo -e \"%s\" > /tmp/d2c.context" % ctxt
        
        action = Action(command=cmd, 
                        sshCred=self.sshCred)
        action.remoteExecutorFactory = self.remoteExecutorFactory
        
        for instance in self.reservation.instances:
            action.execute(instance)
                
    def getPrivateIPs(self):
        '''
        Return a collection of the Private IPs of the instances associated with the role.
        '''
        
        if self.reservation is None:
            self.reservation = self.__getReservation()
        
        for instance in self.reservation.instances: 
            instance.update()
        
        return [str(i.private_ip_address) if i.private_ip_address is not None else str(i.private_dns_name) 
                    for i in self.reservation.instances]
        
    def executeStartCommands(self, shellVars):
        '''
        Execute the start action(s) on each instance within the role.
        '''
        self.__executeActions(self.uploadActions, shellVars)
        self.__executeActions(self.startActions, shellVars)
        self.__executeActions(self.asyncStartActions, shellVars)
                
    def executeStopCommands(self):
        '''
        Execute the end action(s) on each instance within the role.
        '''
          
        self.__executeActions(self.stopActions) 
                
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
                    return False
                
        self.logger.write("Role " + self.template.name + " Returning true for finished test")        
        
        return True
        
    def __getReservation(self):
        '''
        Update the reservation field with current information from AWS.
        '''    
        
        assert hasattr(self, 'reservationId') and self.reservationId is not None
        
        reservations = self.deployment.cloud.getConnection().get_all_instances(filters={'reservation-id':[self.reservationId]})
        
        assert len(reservations) == 1, "Unable to retrieve reservation %s" % self.reservationId
        return reservations[0] 
        
    def collectData(self):
        
        if self.reservation is None:
            self.reservation = self.__getReservation()
        
        for collector in self.dataCollectors:
            for instance in self.reservation.instances:
                
                dest = os.path.join(self.getDataDirectory(), str(instance.id), collector.source[1:])
                
                self.logger.write("Downloading data from instance %s to %s... " % (instance.id, dest))
                collector.collect(instance, 
                                  dest)
                self.logger.write("Done")
    
    def shutdown(self):
        
        if self.reservation is None:
            self.reservation = self.__getReservation()
        
        #Request the instances be terminated  
        for instance in self.reservation.instances:
            instance.stop()
            #Boto 2.0
            #instance.terminate()
        
        #Monitor until all are terminated
        monitorInstances = self.reservation.instances
       
        while len(monitorInstances) > 0:
            for instance in self.reservation.instances:
                instance.update()
                  
            monitorInstances = filter(lambda inst: inst.state != 'terminated', monitorInstances) 
            
            #for instance in self.reservation.instances:
            #    instance.terminate()
                
            time.sleep(self.pollRate)
        
        self.logger.write("Reservation %s successfully terminated" % self.reservation.id)
        
    def __str__(self):
        return "{id:%s, image: %s}" % (self.id, self.image)

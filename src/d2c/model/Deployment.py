'''
Created on Mar 14, 2011

@author: willmore
'''

from threading import Thread
from d2c.logger import StdOutLogger
import time
import subprocess

class Instance:
    '''
    Minimum storage of locally stored instance information. The rest of the instance attributes 
    should be fetched dynamically from AWS via boto.
    '''
    
    def __init__(self, id):
        self.id = id
        
    def __str__(self):
        return "{id: %s}" % self.id
    
class Role:
    
    def __init__(self, deploymentId, 
                 name, ami, count, reservationId=None,
                 startActions = [], 
                 logger=StdOutLogger(), ec2ConnFactory=None):
        
        self.deploymentId = deploymentId
        self.name = name
        self.ami = ami
        self.logger = logger
        
        assert count > 0, "Count must be int > 0"
        self.count = count
        
        self.startActions = list(startActions)
        self.ec2ConnFactory = ec2ConnFactory
        self.reservationId = reservationId
        self.reservation = None
      
    def setEC2ConnFactory(self, ec2ConnFactory):
        self.ec2ConnFactory = ec2ConnFactory  
        
    def launch(self):
        
        self.logger.write("Reserving %d instance(s) of %s" % (self.count, self.ami.amiId))
       
        ec2Conn = self.ec2ConnFactory.getConnection()
        self.reservation = ec2Conn.run_instances(self.ami.amiId, min_count=self.count, 
                                            max_count=self.count, instance_type='t1.micro') #TODO unhardcode instance type
        
        self.logger.write("Instance(s) reserved")    
        
    def executeStartCommands(self):
        '''
        Execute the start action(s) on each instance within the role.
        '''
        
        if self.reservation is None:
            self.reservation = self.__getReservation()
        
        for instance in self.reservation.instances: 
            for action in self.startActions:

                addr = 'ec2-user@%s' % instance.public_dns_name
                #ssh -i cloudeco.pem ec2-user@ec2-46-51-147-10.eu-west-1.compute.amazonaws.com 
                key = '/home/willmore/cert/cloudeco.pem'
                popenCmd = ['rsh', '-i', key, addr]
                popenCmd.extend(action.command.split(" "))
                proc = subprocess.Popen(popenCmd)
                proc.communicate()
                if proc.returncode != 0:
                    raise Exception("Exception exec'ing rsh")
        
    def __getReservation(self):
        '''
        Update the reservation field with current information from AWS.
        '''    
        
        reservations = self.ec2ConnFactory.getConnection().get_all_instances(filters={'reservation-id':[self.reservationId]})
        
        assert len(reservations) == 1, "Unable to retrieve reservation %s" % self.reservationId
        return reservations[0] 
        
    def __str__(self):
        return "{name:%s, ami: %s, instances: %s}" % (self.name, self.ami, str(self.instances))
        
class StartAction:
    
    def __init__(self, role, script):
        self.role = role
        self.script = script
        
class DataCollection:
    
    def __init__(self, role, directory):
        self.role = role
        self.directory = directory

class Monitor(Thread):
    
    class Event:
        
        def __init__(self, newState, deployment):
            self.newState = newState
            self.deployment = deployment
            
    
    def __init__(self, deployment, pollRate=15):
        
        Thread.__init__(self)
        
        self.listeners = {}
        self.deployment = deployment
        self.pollRate = pollRate
        self.currState = self.deployment.state
        self.monitor = True
        self.allStateListeners = []
         
    def addStateChangeListener(self, state, listener):
        
        if not self.listeners.has_key(state):
            self.listeners[state] = []
        
        self.listeners[state].append(listener) 
        
    def addStateAnyChangeListener(self, listener):
        '''
        Add a listener that will be notified of any state change.
        '''
        self.allStateListeners.append(listener)
        
    def stop(self):
        self.monitor = False
        
    def run(self):
        '''
        Launches a poller which detects state changes
        of the deployment and notifies listeners of changes.
        ''' 
    
        while self.monitor:
        
            dState = self.deployment.state
        
            if dState is not self.currState:
                
                self.currState = dState  
                self.notify(dState)
                
            time.sleep(self.pollRate)
            
    def notify(self, state):
        evt = Monitor.Event(state, self.deployment)    
                
        if self.listeners.has_key(state):
            for l in self.listeners[state]:
                l.notify(evt)
                
            for l in self.allStateListeners:
                l.notify(evt)

class DeploymentState:
    
    NOT_RUN = 'NOT_RUN'
    INSTANCES_LAUNCHED = 'INSTANCES_LAUNCHED'
    ROLES_STARTED = 'ROLES_STARTED'
    JOB_COMPLETED = 'JOB_COMPLETED'
    COLLECTING_DATA = 'COLLECTING_DATA'
    DATA_COLLECTED = 'DATA_COLLECTED'
    SHUTTING_DOWN = 'SHUTTING_DOWN'
    COMPLETED = 'COMPLETED'
          

class Deployment(Thread):
    """
    Represents an instance of a Deployment.
    A deployment consists of one or more reservations, 
        which may be in various states (requested, running, terminated, etc.)
    """   
    
    def __init__(self, id, ec2ConnFactory=None, roles=[],
                 reservations=(), state=DeploymentState.NOT_RUN, logger=StdOutLogger(), pollRate=30):
        
        Thread.__init__(self)
        
        self.id = id
        self.ec2ConnFactory = ec2ConnFactory
        self.roles = list(roles)
        
        self.state = state
        self.monitor = Monitor(self, pollRate)
        self.logger = logger
        self.pollRate = pollRate
    
    def setEC2ConnFactory(self, ec2ConnFactory):
        assert self.ec2ConnFactory is None
                
        self.ec2ConnFactory = ec2ConnFactory
    
    def getState(self):
        pass
    
    def addRole(self, role):
        print "Calling add role to " + str(self.roles)
        self.roles.append(role)
        
    def stop(self):
        self.runLifecycle = False
        
        # Allow the monitor to catch any changes in cleanup phase.
        self.join()
        #self.monitor.stop()
        
    def run(self):
        '''
        Run through the lifecycle of the deployment.
        Each stage transition (method) is responsible for
        any cleanup of failures that may occur. Additionally, each stage must monitor
        runLifecycle flag which indicates is the process must stop.
        Each stage must throw an exception if a failure occurs.
        '''
        self.runLifecycle = True
        
        #self.monitor.start()
        
        for step in (self.__launchInstances,
                     self.__startRoles,
                     self.__monitorForDone,
                     self.__collectData,
                     self.__shutdown):
            
            if not self.runLifecycle:
                return
            
            step()
    
    def __launchInstances(self):
        if self.state != DeploymentState.NOT_RUN:
            raise Exception("Can not start deployment in state: " + self.state)
        
        self.logger.write("Launching instances")
        
        for role in self.roles:
            role.setEC2ConnFactory(self.ec2ConnFactory)
            role.launch()    
        
        reservationIds = [r.reservation.id for r in self.roles]
    
        allRunning = False
        
        while self.runLifecycle and not allRunning:
            time.sleep(self.pollRate)
            
            instStates = self.__getInstanceStates(reservationIds)
            
            allRunning = True
            for state in instStates:
                if state != 'running':
                    self.logger.write('All instances not running; continue polling.')
                    allRunning = False
                    break
                
            self.logger.write("All instances now running")
        
        self.__setState(DeploymentState.INSTANCES_LAUNCHED)   
        self.logger.write("Instances Launched")
        self.logger.write("Running start actions")
    
    def __setState(self, state):
        self.state = state
        self.monitor.notify(state)
        
    def __getInstanceStates(self, reservationIds):
        '''
        Return a iterable of string states for all instances
        for the reservation ids.
        '''  
        self.logger.write("Getting instances states for reservation-id(s): %s" % str(reservationIds))
        
        res = self.ec2ConnFactory.getConnection().get_all_instances(filters={'reservation-id':reservationIds})
        
        self.logger.write("Got reservations: %s" % str(res))
        
        states = []
        for r in res:
            for i in r.instances:
                states.append(i.state)
        
        self.logger.write("Instance states for reservations %s are %s" % (str(reservationIds), str(states)))
        
        return states
        
        
    def __startRoles(self):
        self.logger.write("Starting roles")
        
        for role in self.roles:
            role.start()
            
        self.__setState(DeploymentState.ROLES_STARTED)
            
        self.logger.write("Roles started")
        
    def __monitorForDone(self):
        
        self.__setState(DeploymentState.JOB_COMPLETED)
    
    def __collectData(self):  
        self.__setState(DeploymentState.COLLECTING_DATA)
        
        self.__setState(DeploymentState.DATA_COLLECTED)
        
    def __shutdown(self): 
        self.__setState(DeploymentState.SHUTTING_DOWN)
        
        self.__setState(DeploymentState.COMPLETED)
    

    def stopMonitoring(self):
        '''
        Does not end the deployment lifecycle, but detaches monitoring (stops monitoring thread)
        '''
        self.monitor.stop()
    
    def addAnyStateChangeListener(self, listener):
        self.monitor.addStateAnyChangeListener(listener)
        
    def addStateChangeListener(self, state, listener):
        self.monitor.addStateChangeListener(state, listener)
     
    def setPollRate(self, pollRate): 
        self.pollRate = pollRate
        self.monitor.pollRate = pollRate
        
    def __str__(self):
        return "{id:%s, roles:%s}" % (self.id,str(self.roles))
'''
Created on Mar 14, 2011

@author: willmore
'''

from threading import Thread
from d2c.logger import StdOutLogger
import time

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
                 name, ami, count, instances=[],
                 startActions = [], 
                 logger=StdOutLogger(), ec2ConnFactory=None):
        
        self.deploymentId = deploymentId
        self.name = name
        self.ami = ami
        self.logger = logger
        
        assert count > 0
        self.count = count
        
        self.startActions = list(startActions)
        self.ec2ConnFactory = ec2ConnFactory
        self.instances = list(instances)
      
    def setEC2ConnFactory(self, ec2ConnFactory):
        self.ec2ConnFactory = ec2ConnFactory  
        
    def launch(self):
        
        self.logger.write("Reserving %d instance(s) of %s" % (self.count, self.ami.amiId))
       
        ec2Conn = self.ec2ConnFactory.getConnection()
        reservation = ec2Conn.run_instances(self.ami.amiId, min_count=self.count, 
                                      max_count=self.count, instance_type='t1.micro') #TODO unhardcode instance type
        
        self.instances = reservation.instances
        
        self.logger.write("Instance(s) reserved")    
        
    def start(self):
        '''
        Execute the start action(s) on each instance within the role.
        '''
        for instance in self.instances:
            print "Action!"
        
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
    
    
    def __init__(self, deployment, ec2ConnFactory, pollRate=15):
        
        Thread.__init__(self)
        
        self.listeners = {}
        self.deployment = deployment
        self.ec2ConnFactory = ec2ConnFactory
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
        Launches a poller which checks the remote state
        of the deployment and notifies listeners of changes.
        '''
        '''
        self.ec2Conn = self.ec2ConnFactory.getConnection()
        instanceIds = map(lambda i: i.id, self.deployment.getInstances())
        self.instanceHandles = self.ec2Conn.get_all_instances(instanceIds)
        '''
            
        
        
        '''
        Instance states = 
        pending | running | shutting-down | terminated | stopping | stopped
        '''

        while self.monitor:
            newState = DeploymentState.RUNNING

            instanceHandles = []
            
            for role in self.deployment.roles:
                instanceHandles.extend(role.instances)

            for instance in instanceHandles:
                instance.update() # retrieve remote info
                if 'pending' == instance.state:
                    newState = DeploymentState.PENDING
                    break
        
            if newState is not self.currState:
                evt = Monitor.Event(self.currState, self.deployment)
                
                self.currState = newState
                if self.listeners.has_key(self.currState):
                    for l in self.listeners[self.currState]:
                        l.notify(evt)
                
                for l in self.allStateListeners:
                    l.notify(evt)
                
            time.sleep(self.pollRate)

class DeploymentState:
    NOT_RUN = 'NOT_RUN'
    PENDING = 'PENDING'
    RUNNING = 'RUNNING'
    FINALIZING = 'FINALIZING'
    COMPLETED = 'COMPLETED'       

class Deployment:
    """
    Represents an instance of a Deployment.
    A deployment consists of one or more reservations, 
        which may be in various states (requested, running, terminated, etc.)
    """   
    
    def __init__(self, id, ec2ConnFactory=None, roles=[],
                 reservations=(), state=DeploymentState.NOT_RUN, logger=StdOutLogger()):
        
        self.id = id
        self.ec2ConnFactory = ec2ConnFactory
        self.roles = list(roles)
        
        self.state = state
        self.monitor = Monitor(self, self.ec2ConnFactory) if self.ec2ConnFactory is not None else None
        self.logger = logger
    
    def setEC2ConnFactory(self, ec2ConnFactory):
        assert self.ec2ConnFactory is None
        assert self.monitor is None
        
        self.ec2ConnFactory = ec2ConnFactory
        self.monitor = Monitor(self, self.ec2ConnFactory)
    
    def getState(self):
        pass
    
    def addRole(self, role):
        print "Calling add role to " + str(self.roles)
        self.roles.append(role)
        
    def run(self):
        
        if self.state != DeploymentState.NOT_RUN:
            raise Exception("Can not start deployment in state: " + self.state) 
        
        self.monitor.start()
        
        self.logger.write("Launching instances")
        
        for role in self.roles:
            role.setEC2ConnFactory(self.ec2ConnFactory)
            role.launch()    
            
        self.logger.write("Instances Launched")
        self.logger.write("Running start actions")
        
        for role in self.roles:
            role.start()
            
        self.logger.write("Start actions run")
        
        
    def __executeAction(self, action):
        #TODO
        pass

    def stopMonitoring(self):
        '''
        Does not end the deployment lifecycle, but detaches monitoring (stops monitoring thread)
        '''
        self.monitor.stop()
    
    def addAnyStateChangeListener(self, listener):
        self.monitor.addStateAnyChangeListener(listener)
        
    def addStateChangeListener(self, state, listener):
        self.monitor.addStateChangeListener(state, listener)
     
    def setMonitorPollRate(self, pollRate): 
        self.monitor.pollRate = pollRate
        
    def __str__(self):
        return "{id:%s, roles:%s}" % (self.id,str(self.roles))
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
    
    def __init__(self, deploymentId, name, ami, count, instances=None):
        
        self.deploymentId = deploymentId
        self.name = name
        self.ami = ami
        
        assert count > 0
        self.count = count
        
        self.instances = instances if instances is not None else ()
        
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
        self.ec2Conn = self.ec2ConnFactory.getConnection()
        instanceIds = map(lambda i: i.id, self.deployment.getInstances())
        self.instanceHandles = self.ec2Conn.get_all_instances(instanceIds)
        
        '''
        Instance states = 
        pending | running | shutting-down | terminated | stopping | stopped
        '''

        while self.monitor:
            newState = DeploymentState.RUNNING

            for instance in self.instanceHandles:
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
    
    def __init__(self, id, ec2ConnFactory=None, roles=[], startActions=(), dataCollections=(), 
                 reservations=(), state=DeploymentState.NOT_RUN, logger=StdOutLogger()):
        
        self.id = id
        self.ec2ConnFactory = ec2ConnFactory
        self.roles = list(roles)
        self.reservations = list(reservations)
        self.startActions = list(startActions)
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
        
        self.logger.write("Getting EC2Connection")
        ec2Conn = self.ec2ConnFactory.getConnection()
        self.logger.write("Got EC2Connection")
        
        self.reservations = []
        
        for role in self.roles:
            self.reservations.append(self.__reserveInstance(ec2Conn, role))    

    def __reserveInstance(self, ec2Conn, role):
        
        self.logger.write("Reserving %d instance(s) of %s" % (role.count, role.ami.amiId))
       
        r = ec2Conn.run_instances(role.ami.amiId, min_count=role.count, 
                                      max_count=role.count, instance_type='t1.micro') #TODO unhardcode instance type
        
        self.logger.write("Instance(s) reserved")
        
        return r
    
    def stopMonitoring(self):
        '''
        Does not end the deployment, but detaches monitoring (stops monitoring thread)
        '''
        self.monitor.stop()

    def getInstances(self):
        '''
        Return iterable all instances (if any) associated with this deployment
        '''
        out = []
        for l in map(lambda role: role.instances, self.roles):
            out.extend(l)
            
        return out
    
    def addAnyStateChangeListener(self, listener):
        self.monitor.addStateAnyChangeListener(listener)
        
    def addStateChangeListener(self, state, listener):
        self.monitor.addStateChangeListener(state, listener)
     
    def setMonitorPollRate(self, pollRate): 
        self.monitor.pollRate = pollRate
    def __str__(self):
        return "{id:%s, roles:%s}" % (self.id,str(self.roles))
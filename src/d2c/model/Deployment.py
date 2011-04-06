'''
Created on Mar 14, 2011

@author: willmore
'''

from threading import Thread
import time

class Instance:
    '''
    Minimum storage of locally stored instance information. The rest of the instance attributes 
    should be fetched dynamically via boto.
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


class DeploymentState:
    NOT_RUN = 'NOT_RUN'
    PENDING = 'PENDING'
    RUNNING = 'RUNNING'
    FINALIZING = 'FINALIZING'
    COMPLETED = 'COMPLETED'
    
class StartAction:
    
    def __init__(self, role, script):
        self.role = role
        self.script = script
        
class DataCollection:
    
    def __init__(self, role, directory):
        self.role = role
        self.directory = directory

class Monitor(Thread):
    
    def __init__(self, deployment, ec2ConnFactory, pollRate=15):
        
        Thread.__init__(self)
        
        self.listeners = {}
        self.deployment = deployment
        self.ec2ConnFactory = ec2ConnFactory
        self.pollRate = pollRate
        self.currState = self.deployment.state
        self.monitor = True
         
    def addStateChangeListener(self, state, listener):
        
        if not self.listeners.has_key(state):
            self.listeners[state] = []
        
        self.listeners[state].append(listener) 
        
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
                self.currState = newState
                if self.listeners.has_key(self.currState):
                    for l in self.listeners[self.currState]:
                        l.notify()
            
            time.sleep(self.pollRate)
        

class Deployment:
    """
    Represents an instance of a Deployment.
    A deployment consists of one or more reservations, 
        which may be in various states (requested, running, terminated, etc.)
    """   
    
    def __init__(self, id, roles=[], startActions=(), dataCollections=(), 
                 reservations=(), state=DeploymentState.NOT_RUN):
        self.id = id
        self.roles = list(roles)
        self.reservations = list(reservations)
        self.startActions = list(startActions)
        self.state = state
        self.monitor = None
    
    def getState(self):
        pass
    
    def addRole(self, role):
        print "Calling add role to " + str(self.roles)
        self.roles.append(role)
        
    def getInstances(self):
        '''
        Return iterable all instances (if any) associated with this deployment
        '''
        out = []
        for l in map(lambda role: role.instances, self.roles):
            out.extend(l)
            
        return out
    
    def getMonitor(self, ec2ConnFactory, pollRate):
        
        if self.monitor is None:
            self.monitor = Monitor(self, ec2ConnFactory, pollRate)
            
        return self.monitor
        
    def __str__(self):
        return "{id:%s, roles:%s}" % (self.id,str(self.roles))
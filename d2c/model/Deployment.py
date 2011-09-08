
from d2c.logger import StdOutLogger
import time
from .SSHCred import SSHCred
from copy import copy
import string
import random
import os

class Instance(object):
    '''
    Minimum storage of locally stored instance information. The rest of the instance attributes 
    should be fetched dynamically from AWS via boto.
    '''
    
    def __init__(self, id):
        self.id = id
        
    def __str__(self):
        return "{id: %s}" % self.id
    
        
class DataCollection:
    
    def __init__(self, role, directory):
        self.role = role
        self.directory = directory
        

class Monitor:
    '''
    A passive monitor that is notified of deployment events.
    '''
     
    class Event:
        
        def __init__(self, newState, deployment):
            self.newState = newState
            self.deployment = deployment
            
    
    def __init__(self, deployment, listeners={}, pollRate=15):
           
        assert deployment is not None   
                
        self.listeners = dict(listeners)
        self.deployment = deployment
        self.pollRate = pollRate
        self.currState = self.deployment.state
        self.allStateListeners = []
         
    def addStateChangeListener(self, state, listener):
        
        if not self.listeners.has_key(state):
            self.listeners[state] = []
        
        self.listeners[state].append(listener) 
        
    def addStateAnyChangeListener(self, listener):
        '''
        Add a Listener that will be notified of any Deployment state change.
        '''
        self.allStateListeners.append(listener)
            
    def notify(self, state):
        '''
        Notify any registered listeners of the state change.
        '''
        
        evt = Monitor.Event(state, self.deployment)    
        if self.listeners.has_key(state):
            for l in self.listeners[state]:
                l.notify(evt)
                
        for l in self.allStateListeners:
            l.notify(evt)

class DeploymentState:
    
    NOT_RUN = 'NOT_RUN'
    LAUNCHING_INSTANCES = 'LAUNCHING_INSTANCES'
    INSTANCES_LAUNCHED = 'INSTANCES_LAUNCHED'
    ROLES_CONTEXTUALIZED = 'ROLES_CONTEXTUALIZED'
    ROLES_STARTED = 'ROLES_STARTED'
    JOB_COMPLETED = 'JOB_COMPLETED'
    INSTANCES_STOPPED = 'INSTANCES_STOPPED'
    COLLECTING_DATA = 'COLLECTING_DATA'
    DATA_COLLECTED = 'DATA_COLLECTED'
    SHUTTING_DOWN = 'SHUTTING_DOWN'
    COMPLETED = 'COMPLETED'


class StateEvent(object):
    '''
    Records when a Deployment entered a state.
    '''
    
    def __init__(self, state, time):
        self.state = state
        self.time = time

class Deployment(object):
    '''
    Represents an instance of a Deployment.
    A deployment consists of one or more reservations, 
        which may be in various states (requested, running, terminated, etc.)
    '''  
    
    POLL_RATE = 1
    
    def __init__(self, 
                 id, 
                 dataDir,
                 awsCred=None,
                 cloud=None,
                 roles=(),
                 state=DeploymentState.NOT_RUN, 
                 listeners=(),
                 logger=StdOutLogger(), 
                 pollRate=15,
                 deploymentTemplate=None,
                 stateEvents=(),
                 problemSize=0):
                    
        assert isinstance(dataDir, basestring) and len(dataDir) > 0
            
        self.id = id
        self.dataDir = dataDir
        self.cloud = cloud
        self.roles = list()
        self.addRoles(roles)
        self.awsCred = awsCred
        self.deploymentTemplate = deploymentTemplate
        
        self.state = state
        
        self.logger = logger
        self.pollRate = pollRate
        
        self.stopInstances = False
        self.stateEvents = list(stateEvents)
    
        self.problemSize = problemSize
        
    def getMaxMemory(self):
        return self.maxMemory
    
    def setLogger(self, logger, cascade=True):
        self.logger = logger
        if cascade:
            self._cascadeLogger()
        
        
    def _cascadeLogger(self):
        
        for role in self.roles:
            role.setLogger(self.logger)
    
    def hasCompleted(self):
        return self.state == DeploymentState.JOB_COMPLETED
    
    def roleRunTime(self):
        startTime = 0
        endTime = 0
        
        for e in self.stateEvents:
            if e.state == DeploymentState.ROLES_STARTED:
                startTime = e.time
            elif e.state == DeploymentState.JOB_COMPLETED:
                endTime = e.time
                
        return endTime - startTime
    
    def setCloud(self, cloud):

        if self.cloud is not cloud:
            self.cloud = cloud
            
        if self not in cloud.deployments:
            cloud.deployments.append(self)
          
    def addRoles(self, roles):
        for r in roles:
            self.addRole(r)

    def addRole(self, role):
        print "Add role"
        if role not in self.roles:
            print "Adding role"
            self.roles.append(role)
            
        if self is not role.deployment:
            role.deployment = self
       
    def costPerHour(self):
        sum = 0
        for r in self.roles:
            sum += r.costPerHour()
            
        return sum
        
    def pause(self):
        '''
        Pauses lifecycle management.
        This will NOT stop or terminate any running instances.
        '''
        self.runLifecycle = False
        
    def stop(self):
        '''
        Stop any running instances and end lifecycle.
        '''
        self.runLifecycle = False
        self.stopInstances = True
        self.logger.write("Canceling deployment")
    
    def _getMonitor(self):
        
        if not hasattr(self, 'monitor'):
            self.monitor = Monitor(self, pollRate=Deployment.POLL_RATE)

        return self.monitor
        
    def run(self):
        '''
        Run through the life-cycle of the deployment.
        Each stage transition (method) is responsible for
        any cleanup of failures that may occur. Additionally, each stage must monitor
        runLifecycle flag which indicates is the process must stop.
        Each stage must throw an exception if a failure occurs.
        '''
        self.runLifecycle = True
        
        #Ordered mapping of existing stage to transition.
        #Used for restarting an already running deployment.
    
        self.logger.write("Starting deployment")
    
        #TODO Rework state model - make it more OO
        stageList = ((DeploymentState.NOT_RUN, None),
                     (DeploymentState.LAUNCHING_INSTANCES, self.__launchInstances),
                     (DeploymentState.INSTANCES_LAUNCHED, self.__contextualize),
                     (DeploymentState.ROLES_CONTEXTUALIZED, self.__startRoles),
                     (DeploymentState.ROLES_STARTED, self.__monitorForDone),
                     (DeploymentState.JOB_COMPLETED, self.__stopRoles),
                     (DeploymentState.INSTANCES_STOPPED, self.__collectData),
                     (DeploymentState.COLLECTING_DATA, None),
                     (DeploymentState.DATA_COLLECTED, self.__shutdown),
                     (DeploymentState.SHUTTING_DOWN, None),
                     (DeploymentState.COMPLETED, None))
        
        startIdx = 0
        for (state, _) in stageList:
            if self.state == state:
                break
            startIdx += 1 
        
        for (state, transition) in stageList[startIdx:]:
            if not self.runLifecycle: # Extra check to see we are still alive.
                if self.stopInstances:
                    self.__shutdown()        
                return
            
            self.stateEvents.append(StateEvent(state, time.time()))
            
            if transition is None:
                continue
            
            transition()
    
    def __launchInstancesParallel(self):
        
        for role in self.roles:
            role.launch(self.awsCred)    
        
        reservationIds = [r.getReservationId() for r in self.roles]
    
        allRunning = False
        
        conn = self.cloud.getConnection(self.awsCred)
        
        while self.runLifecycle and not allRunning:
            time.sleep(Deployment.POLL_RATE)
            
            if not self.runLifecycle:
                return
           
            instStates = conn.getInstanceStates(reservationIds)
            
            allRunning = True
            
            for state in instStates:
                if state != 'running':
                    #self.logger.write('All instances not running; continue polling.')
                    allRunning = False
                    break
                
            
        
    def __launchInstancesSerial(self):
        
        conn = self.cloud.getConnection(self.awsCred)
        
        for role in self.roles:
            role.launch(self.awsCred)    
            
        allRunning = False
        for role in self.roles:
            role.launch(self.awsCred)
            reservationId = role.getReservationId()
            
            while self.runLifecycle and not allRunning:
                time.sleep(Deployment.POLL_RATE)
                
                if not self.runLifecycle:
                    return
               
                instStates = conn.getInstanceStates([reservationId])
                
                allRunning = True
                
                for state in instStates:
                    if state != 'running':
                        #self.logger.write('All instances not running; continue polling.')
                        allRunning = False
                        break
                    
                for role in self.roles:
                    for ip in role.getPrivateIPs():
                        if ip is None or ip is "" or ip is "0.0.0.0":
                            allRunning = False
        
    
    def __launchInstances(self):
        if self.state != DeploymentState.NOT_RUN:
            raise Exception("Can not start deployment in state: " + self.state)
        
        self.__setState(DeploymentState.LAUNCHING_INSTANCES)
        
        self.logger.write("Creating session key")
        
        self.keyPairName = "%s.%s" % (self.deploymentTemplate.name, self.id)
        
        conn = self.cloud.getConnection(self.awsCred)
        
        keyDirLoc = conn.generateKeyPair(self.dataDir, self.keyPairName)
        self.logger.write("Session key created: %s" % keyDirLoc)
        sshCred = SSHCred(id=None, name=self.keyPairName, username="root", privateKey=keyDirLoc)
        
        for role in self.roles:
            role.setSSHCred(sshCred)
        
        self.logger.write("Launching instances")
        
        #TODO move into Cloud class/subclasses              
        #if isinstance(self.cloud, DesktopCloud):
        #self.__launchInstancesSerial()
        #else:
        self.__launchInstancesParallel()
        
                
        self.logger.write("The deployment's instance have booted.")
        '''
        Wait a bit for the systems to really boot up.
        TODO: replace hardcoded wait time with a valid test, perhaps ping.
        '''
        time.sleep(30)
        
        self.__setState(DeploymentState.INSTANCES_LAUNCHED)   
        self.logger.write("Instances Launched")
    
    def __contextualize(self):
        
        self.logger.write("Contextualizing Instances")
        
        ips = []
        for role in self.roles:
            ips.extend(role.getPrivateIPs())
                    
        for role in self.roles:
            role.setIPContext(ips)
            
        self.__setState(DeploymentState.ROLES_CONTEXTUALIZED)
            
        self.logger.write("Instances Contextualized")
    
    def __setState(self, state):
        self.state = state
        self.monitor.notify(state)
        
        
    def __startRoles(self):
        self.logger.write("Starting roles")
                
        self._cascadeLogger()  
        
        for role in self.roles:
            role.executeStartCommands()
            
        self.__setState(DeploymentState.ROLES_STARTED)
            
        self.logger.write("Roles started")
        
    def __stopRoles(self):
        self.logger.write("Stopping roles")
        #        
        #for role in self.roles:
        #    role.executeStopCommands()
        #    
        #self.__setState(DeploymentState.INSTANCES_STOPPED)
            
        self.logger.write("Roles stopped")
        
        
    def __monitorForDone(self):
        
        monitorRoles = list(self.roles)
        self.logger.write("Monitoring instances for done conditions")
        while len(monitorRoles) > 0:
            monitorRoles = [role for role in monitorRoles if not role.checkFinished()]
            #self.logger.write("Monitor role len is %d" % len(monitorRoles))
            time.sleep(self.pollRate)
                
        self.__setState(DeploymentState.JOB_COMPLETED)
    
    def __collectData(self):  
        self.__setState(DeploymentState.COLLECTING_DATA)
        
        self._cascadeLogger()
        
        for role in self.roles:
            role.collectData()
                
        self.__setState(DeploymentState.DATA_COLLECTED)
        
    def __shutdown(self): 
        self.__setState(DeploymentState.SHUTTING_DOWN)
        
        for role in self.roles:
            role.shutdown()
        
        self.__setState(DeploymentState.COMPLETED)
    
    def addAnyStateChangeListener(self, listener):
        self._getMonitor().addStateAnyChangeListener(listener)
        
    def addStateChangeListener(self, state, listener):
        self._getMonitor().addStateChangeListener(state, listener)
     
    def setPollRate(self, pollRate): 
        self.pollRate = pollRate
        self.monitor.pollRate = pollRate
        
    def clone(self):
        '''
        Create a clone of this Deployment and Roles. Id's will be set to None.
        '''
        id = ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(6))
        dataDir = os.path.join(self.deploymentTemplate.dataDir, id)
        c = Deployment(id, dataDir, cloud=self.cloud, deploymentTemplate=self.deploymentTemplate, awsCred=self.awsCred, problemSize=self.problemSize)
        
        c.roles = [r.clone() for r in self.roles]
        
        for r in c.roles:
            r.deployment = c
        
        return c
        
    def __str__(self):
        return "{id:%s, roles:%s}" % (self.id,str(self.roles))

from d2c.logger import StdOutLogger
import time

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
          

class Deployment(object):
    '''
    Represents an instance of a Deployment.
    A deployment consists of one or more reservations, 
        which may be in various states (requested, running, terminated, etc.)
    '''  
    
    def __init__(self, 
                 id, 
                 dataDir,
                 awsCred=None,
                 cloud=None,
                 roles=(),
                 reservations=(), 
                 state=DeploymentState.NOT_RUN, 
                 listeners={},
                 logger=StdOutLogger(), 
                 pollRate=30):
                        
        self.id = id
        self.dataDir = dataDir
        self.cloud = cloud
        self.roles = list()
        self.addRoles(roles)
        self.awsCred = awsCred
        
        self.state = state
        self.monitor = Monitor(self, listeners, pollRate)
        self.logger = logger
        self.pollRate = pollRate
        
        self.stopInstances = False
    
    def setLogger(self, logger, cascade=True):
        self.logger = logger
        if cascade:
            self._cascadeLogger()
        
        
    def _cascadeLogger(self):
        
        for role in self.roles:
            role.setLogger(self.logger)
    
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
        
        for (_, transition) in stageList[startIdx:]:
            if not self.runLifecycle: # Extra check to see we are still alive.
                if self.stopInstances:
                    self.__shutdown()
                
                return
            
            if transition is None:
                continue
            
            transition()
    
    def __launchInstances(self):
        if self.state != DeploymentState.NOT_RUN:
            raise Exception("Can not start deployment in state: " + self.state)
        
        self.__setState(DeploymentState.LAUNCHING_INSTANCES)
        
        self.logger.write("Launching instances")
                        
        for role in self.roles:
            role.launch(self.awsCred)    
        
        reservationIds = [r.getReservationId() for r in self.roles]
    
        allRunning = False
        
        while self.runLifecycle and not allRunning:
            time.sleep(self.pollRate)
            
            if not self.runLifecycle:
                return
           
            instStates = self.__getInstanceStates(reservationIds)
            
            allRunning = True
            for state in instStates:
                if state != 'running':
                    self.logger.write('All instances not running; continue polling.')
                    allRunning = False
                    break
                
        self.logger.write("All instances now running")
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
        
    def __getInstanceStates(self, reservationIds):
        '''
        Return a iterable of string states for all instances
        for the reservation IDs.
        '''  
        
        self.logger.write("Getting instances states for reservation-id(s): %s" % str(reservationIds))
        
        # Filter only works in boto 2.0. Add back when we move from 1.9 to 2.0
        #res = self.cloud.getConnection(self.awsCred).get_all_instances(filters={'reservation-id':reservationIds})
        
        res = self.cloud.getConnection(self.awsCred).get_all_instances()
        
        self.logger.write("Got reservations: %s" % str(res))
        
        states = []
        for r in res:
            if r.id in reservationIds:
                for i in r.instances:
                    states.append(i.state)
        
        self.logger.write("Instance states for reservations %s are %s" % (str(reservationIds), str(states)))
        
        return states
        
        
    def __startRoles(self):
        self.logger.write("Starting roles")
                
        self._cascadeLogger()  
        
        for role in self.roles:
            role.executeStartCommands()
            
        self.__setState(DeploymentState.ROLES_STARTED)
            
        self.logger.write("Roles started")
        
    def __stopRoles(self):
        self.logger.write("Stopping roles")
                
        for role in self.roles:
            role.executeStopCommands()
            
        self.__setState(DeploymentState.INSTANCES_STOPPED)
            
        self.logger.write("Roles stopped")
        
        
    def __monitorForDone(self):
        
        monitorRoles = list(self.roles)
        
        while len(monitorRoles) > 0:
            monitorRoles = [role for role in monitorRoles if not role.checkFinished()]
            self.logger.write("Monitor role len is %d" % len(monitorRoles))
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
        self.monitor.addStateAnyChangeListener(listener)
        
    def addStateChangeListener(self, state, listener):
        self.monitor.addStateChangeListener(state, listener)
     
    def setPollRate(self, pollRate): 
        self.pollRate = pollRate
        self.monitor.pollRate = pollRate
        
    def __str__(self):
        return "{id:%s, roles:%s}" % (self.id,str(self.roles))
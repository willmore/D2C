import unittest
import time
from d2c.model.Deployment import Deployment, DeploymentState
from d2c.model.Role import Role
from d2c.model.AMI import AMI
from d2c.model.Configuration import Configuration
from d2c.model.EC2Cred import EC2Cred
from d2c.data.DAO import DAO
from threading import Thread
from boto.ec2.instance import Reservation
from mockito import *


class DummyConnFactory:
    
    def __init__(self):
        self.conn = DummyConn()
    
    def setState(self, state):
        self.conn.setState(state)
    
    def getConnection(self):
        return self.conn
    
class DummyConn:
    
    def __init__(self):
        self.num = 0
        self.instances = []
        self.reservations = {}
    
    def get_all_instances(self, ids = None, filters={}):

        return filter(lambda r: r.id in filters['reservation-id'], 
                      self.reservations.values())
    
    def run_instances(self, *args, **kwargs):
        r = DummyReservation(kwargs['min_count'])
        self.reservations[r.id] = r
        return r
    
    def setState(self, state):
        for i in self.instances:
            i.state = state 
        
        for r in self.reservations.values():
            r.state = state
            
class DummyReservation:
    
    ctr = 0
    
    def __init__(self, count, id=None):
        self.instances = [DummyInstance(None) for _ in range(count)]
        self.id = id if id is not None else 'r-dummy_%d' % self.ctr 
        self.ctr += self.ctr
    
    def setState(self, state):
        for i in self.instances:
            i.state = state 
        
    def update(self):
        pass

class DummyInstance():
    
    def __init__(self, id):
        self.id = id
        self.state = 'pending'
        self.key_name = 'dummy_key_name'
        
    def update(self):
        pass

class DummyAction:
    
    def __init__(self):
        self.called = False
    
    def execute(self, instance):
        self.called = True
        
class DummyFinishedTest:
    
    def __init__(self):
        self.times = 0
        
    def check(self, instance):
        
        assert instance is not None
        return True

class DeploymentTest(unittest.TestCase):
   
    def setUp(self):
        dName = "Dummy"
        ami = AMI("ami-123", "foobar.vdi")
        dao = mock(DAO)
        conf = mock(Configuration)
        cred = mock(EC2Cred)
        conf.ec2Cred = cred
        when(dao).getEC2Cred().thenReturn(cred)
        when(dao).getConfiguration().thenReturn(conf)
        
        '''
        self.deployment = Deployment(dName, roles = [Role(dName, "loner", ami, 2, dao=dao, 
                                                          startActions=[DummyAction(), DummyAction()],
                                                          finishedChecks=[DummyFinishedTest(), DummyFinishedTest()]), 
                                                     Role(dName, "loner2", ami, 2, dao=dao, 
                                                          startActions=[DummyAction(), DummyAction()],
                                                          finishedChecks=[DummyFinishedTest(), DummyFinishedTest()],
                                                          dataCollectors=[Mock(DataCollector)])])
        '''
        
        roles = []
        for _ in range(3):
            role = mock(Role)
            when(role).checkFinished().thenReturn(True)
            
        self.deployment = Deployment(dName, roles=roles) 
                                                     
        
    def tearDown(self):
        if hasattr(self, 'mon'):
            self.mon.stop()
        
    def assertStartActionsCalled(self, deployment):
        for role in deployment.roles:
            self.assertTrue(role.executeStartCommands.call_count, 1)
                
    def assertStartActionsNotCalled(self, deployment):
        for role in deployment.roles:
            self.assertFalse(role.executeStartCommands.called)
                
    def assertDataCollectorsCalled(self, deployment):
        for role in deployment.roles:
            self.assertTrue(role.collectData.call_count, 1)
        
    def testLifecycle(self):
        '''
        Test listeners are properly notified when deployment goes to running state.
        '''
        pollRate = 2
        connFactory = DummyConnFactory()
            
        self.deployment.cloud = connFactory
        
        
             
        listeners = {}
        
        for state in (DeploymentState.INSTANCES_LAUNCHED, 
                      DeploymentState.ROLES_STARTED,
                      DeploymentState.JOB_COMPLETED,
                      DeploymentState.COLLECTING_DATA,
                      DeploymentState.DATA_COLLECTED,
                      DeploymentState.SHUTTING_DOWN,
                      DeploymentState.COMPLETED):
            l = mock()
            self.deployment.addStateChangeListener(state, 
                                   l)
            listeners[state] = l
        
        self.deployment.setPollRate(pollRate)
        
        thread = Thread(target=self.deployment.run)
        thread.start()
        
        time.sleep(2 * pollRate)
        #Manually set mock instances to running
        connFactory.setState('running')    
        thread.join(30)
        
        for (s,l) in listeners.iteritems():
            print "State is " + s
            verify(l).notify(any())
        
        self.assertStartActionsCalled(self.deployment)
        self.assertDataCollectorsCalled(self.deployment)
        
    def testReAttachInstancesLaunched(self):
        '''
        Simulate re-attaching to an already started deployment, which is at stage INSTANCES_LAUNCHED.
        Assert that the lifecycle properly continues after re-attaching.
        '''
        connFactory = DummyConnFactory()
        self.deployment.cloud = connFactory
        
        
        for role in self.deployment.roles:
            reservationId = 'r-%s' % role.getName()
            role.setReservationId(reservationId)
            connFactory.conn.reservations[reservationId] = DummyReservation(count=role.getCount(), id=reservationId)
        
        connFactory.setState('running')
        pollRate = 2
        hits = {}
                     
        class Listener:
            
            def __init__(self, state):
                self.state = state
            
            def notify(self, evt):
                hits[self.state] = True
          
        for state in (DeploymentState.INSTANCES_LAUNCHED, 
                      DeploymentState.ROLES_STARTED,
                      DeploymentState.JOB_COMPLETED,
                      DeploymentState.COLLECTING_DATA,
                      DeploymentState.DATA_COLLECTED,
                      DeploymentState.SHUTTING_DOWN,
                      DeploymentState.COMPLETED):
            self.deployment.addStateChangeListener(state, 
                                   Listener(state)) #MicroMock(notify=lambda evt:hits.__setitem__(state, True)))
        
        self.deployment.state = DeploymentState.INSTANCES_LAUNCHED
        self.deployment.setPollRate(pollRate)
        
        thread = Thread(target=self.deployment.run)
        thread.start()
        
        time.sleep(2 * pollRate)

        thread.join(15)
        
        self.assertFalse(hits.has_key(DeploymentState.INSTANCES_LAUNCHED)) #Should only hit new stages after this one
        self.assertTrue(hits.has_key(DeploymentState.ROLES_STARTED))
        self.assertTrue(hits.has_key(DeploymentState.JOB_COMPLETED))
        self.assertTrue(hits.has_key(DeploymentState.COLLECTING_DATA))
        self.assertTrue(hits.has_key(DeploymentState.DATA_COLLECTED))
        self.assertTrue(hits.has_key(DeploymentState.SHUTTING_DOWN))
        self.assertTrue(hits.has_key(DeploymentState.COMPLETED))
        self.assertStartActionsCalled(self.deployment)
        
        
    def testReAttachRoleStarted(self):
        '''
        Simulate re-attaching to an already started deployment, which is at stage ROLES_STARTED.
        Assert that the lifecycle properly continues after re-attaching.
        '''
        cloud = DummyConnFactory()
        self.deployment.cloud = cloud
        
        for role in self.deployment.roles:
            reservationId = 'r-%s' % role.getName() 
            cloud.conn.reservations[reservationId] = mock(Reservation)
        
        cloud.setState('running')
        pollRate = 2
        hits = {}
                     
        class Listener:
            
            def __init__(self, state):
                self.state = state
            
            def notify(self, evt):
                hits[self.state] = True
          
        for state in (DeploymentState.INSTANCES_LAUNCHED, 
                      DeploymentState.ROLES_STARTED,
                      DeploymentState.JOB_COMPLETED,
                      DeploymentState.COLLECTING_DATA,
                      DeploymentState.DATA_COLLECTED,
                      DeploymentState.SHUTTING_DOWN,
                      DeploymentState.COMPLETED):
            self.deployment.addStateChangeListener(state, 
                                   Listener(state)) #MicroMock(notify=lambda evt:hits.__setitem__(state, True)))
        
        self.deployment.state = DeploymentState.ROLES_STARTED
        self.deployment.setPollRate(pollRate)
    
        thread = Thread(target=self.deployment.run)
        thread.start()
        
        time.sleep(2 * pollRate)
         
        thread.join(15)
        
        self.assertFalse(hits.has_key(DeploymentState.INSTANCES_LAUNCHED)) #Should only hit new stages after this one
        self.assertFalse(hits.has_key(DeploymentState.ROLES_STARTED))
        self.assertTrue(hits.has_key(DeploymentState.JOB_COMPLETED))
        self.assertTrue(hits.has_key(DeploymentState.COLLECTING_DATA))
        self.assertTrue(hits.has_key(DeploymentState.DATA_COLLECTED))
        self.assertTrue(hits.has_key(DeploymentState.SHUTTING_DOWN))
        self.assertTrue(hits.has_key(DeploymentState.COMPLETED))
        self.assertStartActionsNotCalled(self.deployment)
        
if __name__ == '__main__':
    unittest.main()




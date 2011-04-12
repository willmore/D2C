import unittest
import time
from d2c.model.Deployment import Deployment, DeploymentState, Role
from d2c.model.AMI import AMI
from MicroMock import MicroMock


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
            r.setState(state)
            
class DummyReservation:
    
    ctr = 0
    
    def __init__(self, count, id=None):
        self.instances = [DummyInstance(None) for _ in range(count)]
        self.id = id if id is not None else 'r-dummy_%d' % self.ctr 
        self.ctr += self.ctr
    
    def setState(self, state):
        for i in self.instances:
            print "Setting state"
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
    
    
class DummyDao:
    
    def getEC2Cred(self, id):
        return MicroMock(id=id, cert="dummy_cert", private_key="dummy_private_key")

    def getConfiguration(self):
        return MicroMock(ec2Cred=self.getEC2Cred('dummy_cred_id'))

class DeploymentTest(unittest.TestCase):
   
    def setUp(self):
        dName = "Dummy"
        ami = AMI("ami-123", "foobar.vdi")
        dao = DummyDao()
        self.deployment = Deployment(dName, roles = [Role(dName, "loner", ami, 2, dao=dao), 
                                        Role(dName, "loner2", ami, 2, dao=dao)])
        
    def tearDown(self):
        if hasattr(self, 'mon'):
            self.mon.stop()
        
    def testLifecycle(self):
        '''
        Test listeners are properly notified when deployment goes to running state.
        '''
        pollRate = 2
        hits = {}
        connFactory = DummyConnFactory()
        
        
        self.deployment.setEC2ConnFactory(connFactory)
       
         
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
        
        self.deployment.setPollRate(pollRate)
        
        self.deployment.start()
        
        time.sleep(2 * pollRate)
        #Manually set mock instances to running
        connFactory.setState('running')    
        self.deployment.join(15)
        
        self.assertTrue(hits.has_key(DeploymentState.INSTANCES_LAUNCHED))   
        self.assertTrue(hits.has_key(DeploymentState.ROLES_STARTED))
        self.assertTrue(hits.has_key(DeploymentState.JOB_COMPLETED))
        self.assertTrue(hits.has_key(DeploymentState.COLLECTING_DATA))
        self.assertTrue(hits.has_key(DeploymentState.DATA_COLLECTED))
        self.assertTrue(hits.has_key(DeploymentState.SHUTTING_DOWN))
        self.assertTrue(hits.has_key(DeploymentState.COMPLETED))
        
        
    def testReAttachInstancesLaunched(self):
        '''
        Simulate re-attaching to an already started deployment, which is at stage INSTANCES_LAUNCHED.
        Assert that the lifecycle properly continues after re-attaching.
        '''
        connFactory = DummyConnFactory()
        self.deployment.setEC2ConnFactory(connFactory)
        
        
        for role in self.deployment.roles:
            reservationId = 'r-%s' % role.name 
            role.reservationId = reservationId
            connFactory.conn.reservations[reservationId] = DummyReservation(count=role.count, id=reservationId)
        
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
        
        self.deployment.start()
        
        time.sleep(2 * pollRate)

        self.deployment.join(15)
        
        self.assertFalse(hits.has_key(DeploymentState.INSTANCES_LAUNCHED)) #Should only hit new stages after this one
        self.assertTrue(hits.has_key(DeploymentState.ROLES_STARTED))
        self.assertTrue(hits.has_key(DeploymentState.JOB_COMPLETED))
        self.assertTrue(hits.has_key(DeploymentState.COLLECTING_DATA))
        self.assertTrue(hits.has_key(DeploymentState.DATA_COLLECTED))
        self.assertTrue(hits.has_key(DeploymentState.SHUTTING_DOWN))
        self.assertTrue(hits.has_key(DeploymentState.COMPLETED))
        
        
    def testReAttachRoleStarted(self):
        '''
        Simulate re-attaching to an already started deployment, which is at stage ROLES_STARTED.
        Assert that the lifecycle properly continues after re-attaching.
        '''
        connFactory = DummyConnFactory()
        self.deployment.setEC2ConnFactory(connFactory)
        
        
        for role in self.deployment.roles:
            reservationId = 'r-%s' % role.name 
            role.reservationId = reservationId
            connFactory.conn.reservations[reservationId] = DummyReservation(count=role.count, id=reservationId)
        
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
        
        self.deployment.state = DeploymentState.ROLES_STARTED
        self.deployment.setPollRate(pollRate)
    
        self.deployment.start()
        
        time.sleep(2 * pollRate)
         
        self.deployment.join(15)
        
        self.assertFalse(hits.has_key(DeploymentState.INSTANCES_LAUNCHED)) #Should only hit new stages after this one
        self.assertFalse(hits.has_key(DeploymentState.ROLES_STARTED))
        self.assertTrue(hits.has_key(DeploymentState.JOB_COMPLETED))
        self.assertTrue(hits.has_key(DeploymentState.COLLECTING_DATA))
        self.assertTrue(hits.has_key(DeploymentState.DATA_COLLECTED))
        self.assertTrue(hits.has_key(DeploymentState.SHUTTING_DOWN))
        self.assertTrue(hits.has_key(DeploymentState.COMPLETED))
        
if __name__ == '__main__':
    unittest.main()




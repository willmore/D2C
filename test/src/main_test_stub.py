import sys
import os

from d2c.Application import Application
from d2c.data.DAO import DAO
from d2c.AMITools import AMIToolsFactory, AMITools

from d2c.model.Kernel import Kernel

from mockito import *
import boto
from threading import Thread
import time
import test_initor
    
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
        
        class RunThread(Thread):
        
            def __init__(self, dummyConn):
                Thread.__init__(self)
                self.dummyConn = dummyConn
            
            def run(self):
                print "Starting thread"
                for state in ['running', 'terminated']:
                    time.sleep(60)
                    self.dummyConn.setState(state)
        
        RunThread(self).start()
        return r
    
    def setState(self, state):
        
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
            i.state = state 
        
    def update(self):
        pass

class DummyInstance():
    
    def __init__(self, id):
        self.id = id
        self.state = 'pending'
        self.key_name = 'dummy_key_name'
        self.private_ip_address = "0.0.0.0"
        
    def update(self):
        pass

    def stop(self):
        pass

def main(argv=None):
    
    sqlFile = "%s/.d2c_test/main_test_stub.sqlite" % os.path.expanduser('~') 
    if os.path.exists(sqlFile):
        print "Deleting existing DB"
        os.unlink(sqlFile)
        
    mockBoto = mock(boto)
    #when(mockBoto).connect_ec2(any(),any(),any(),any(),any(),any()).thenReturn(DummyConn())
    
    def mock_connect_ec2(*args, **kwargs):
        return DummyConn()
    
    mockBoto.connect_ec2 = mock_connect_ec2
   
    dao = DAO(sqlFile, mockBoto)
    
    test_initor.init_db(dao)
    
    mockAMIFactory = mock(AMIToolsFactory)
    mockAMITools = mock(AMITools)
    when(mockAMIFactory).getAMITools(any()).thenReturn(mockAMITools)
    when(mockAMITools).getArch(any()).thenReturn(Kernel.ARCH_X86_64)
    when(mockAMITools).registerAMI(any(), any(), any()).thenReturn("foobarami")
    
    print dao.getAMIs()
    
    app = Application(dao, mockAMIFactory)
    app.MainLoop()
    


if __name__ == "__main__":
    sys.exit(main())

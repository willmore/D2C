'''
Created on Apr 6, 2011

@author: willmore
'''


class Launcher():
    '''
    classdocs
    '''


    def __init__(self, ec2ConnFactory):
        '''
        Constructor
        '''
        self.ec2ConnFactory = ec2ConnFactory
        
    def launch(self, deployment):
        conn = self.ec2ConnFactory.getConnection()
        
        for role in deployment.roles:
            self.__launchRole(conn, role)
            
        
        
    def __launchRole(self, conn, role):
        
        image = conn.get_image(role.ami.amiId)
        
        reservation = image.run(min_count=role.count, max_count=role.count)
        print "Reservation is %s" % reservation
        
'''
Created on Apr 6, 2011

@author: willmore
'''

from d2c.model.Deployment import Instance

class Launcher():
    '''
    classdocs
    '''

    def __init__(self, ec2ConnFactory, dao):
        '''
        Constructor
        '''
        self.ec2ConnFactory = ec2ConnFactory
        self.dao = dao
        
    def launch(self, deployment):
        conn = self.ec2ConnFactory.getConnection()
        
        instances = []
        
        for role in deployment.roles:
            instances.extend(self.__launchRole(conn, role))
            
        return instances
        
        
    def __launchRole(self, conn, role):
        '''
        Launch instances specified by role's ami and count.
        Update the DB with the role's instance(s).
        Return the boto instance objects associated with the role.
        '''
        image = conn.get_image(role.ami.amiId)
        
        reservation = image.run(min_count=role.count, max_count=role.count)
        
        role.instances = []
        for i in reservation.instances:
            self.dao.addRoleInstance(role.deploymentId, role.name, i.id)
            role.instances.append(Instance(i.id))
        
        return reservation.instances
        
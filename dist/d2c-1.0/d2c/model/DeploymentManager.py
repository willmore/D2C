'''
Created on Mar 14, 2011

@author: willmore
'''

from .Deployment import Deployment

class DeploymentManager:

    def __init__(self, dao, ec2ConnectionFactory, logger):
        self.dao = dao
        self.ec2ConnectionFactory = ec2ConnectionFactory
        self.logger = logger
    
    def reserveInstance(self, ec2Conn, role, count):
        
        self.logger.write("Reserving %d instance(s) of %s" % (count, role))
       
        r = ec2Conn.run_instances(role.ami.amiId, min_count=count, 
                                      max_count=count, instance_type='m1.large') #TODO unhardcode instance type
        
        self.logger.write("Instance(s) reserved")
        
        return r
    
    def deploy(self, deploymentDesc):
        
        self.logger.write("Getting EC2Connection")
        ec2Conn = self.ec2ConnectionFactory.getConnection()
        self.logger.write("Got EC2Connection")
        
        reservations = []
        
        for (role, count) in deploymentDesc.roleCountMap.iteritems():
            reservations.append(self.reserveInstance(ec2Conn, role, count))
            
        return Deployment(deploymentDesc, reservations)
  
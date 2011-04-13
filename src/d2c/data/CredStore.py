'''
Created on Apr 13, 2011

@author: willmore
'''


class CredStore:
    
    def __init__(self, dao):
        assert dao is not None
        
        self.dao = dao
        
    def getEC2Cred(self, id):
        return self.dao.getEC2Cred(id)
    
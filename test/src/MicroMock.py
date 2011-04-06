'''
Created on Apr 6, 2011

@author: willmore
'''

class MicroMock(object):
    
    def __init__(self, **kwargs):
        self.__dict__.update(kwargs)

        
from distutils.core import setup

D2C_PKGLIST = ['d2c',
               'd2c.aws',
               'd2c.controller',
               'd2c.data',
               'd2c.gui',
               'd2c.model']
    

setup(name='d2c',
      version='1.0',
      packages=D2C_PKGLIST
      )
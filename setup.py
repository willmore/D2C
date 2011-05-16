from distutils.core import setup

D2C_PKGLIST = ['d2c',
               'd2c.aws',
               'd2c.controller',
               'd2c.data',
               'd2c.gui',
               'd2c.model']
    

setup(name='d2c',
      version='1.0',
      packages=D2C_PKGLIST,
      scripts=['bin/d2c_gui'],
      data_files=[("data/kernels", 
                    ["data/kernels/2.6.35-24-virtual.tar", "data/kernels/2.6.35-24-virtual-x86_64.tar"]),
                  ("data/gui/icons", ["data/icons/alien.png"])]
      )

from distutils.core import setup

D2C_PKGLIST = ['d2c',
               'd2c.aws',
               'd2c.controller',
               'd2c.data',
               'd2c.gui',
               'd2c.model',
               'd2c.graph']
    

setup(name='d2c',
      version='1.0',
      packages=D2C_PKGLIST,
      scripts=['bin/d2c_gui', 'bin/stat_viewer'],
      package_data={'d2c': ['ami_data/fstab', 'ami_data/kernels/*.tar'],
                    'd2c.gui': ['icons/*.png']},
      )

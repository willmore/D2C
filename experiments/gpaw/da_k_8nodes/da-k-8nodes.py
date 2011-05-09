from ase import *
from ase.parallel import paropen
from gpaw import *
from math import *

sqrt = 1.732050808

def energy(k, h, dl, da, vac, var, num):
    slab = Atoms([Atom('Bi', (0.0000, 0.0000,          -dl)),
                  Atom('Bi', (da/2,   sqrt*da/6,    0.0)),
                  Atom('Bi', (da/2,   sqrt*da/2,    -dl)),
                  Atom('Bi', (2*da,   2*sqrt*da/3,  0.0)),
                  Atom('Bi', (da,     0.0,             -dl)),
                  Atom('Bi', (3*da/2, sqrt*da/6,    0.0)),
                  Atom('Bi', (3*da/2, sqrt*da/2,    -dl)),
                  Atom('Bi', (da,     2*sqrt*da/3,  0.0))],
                  cell=((2*da,0,0), (da,sqrt*da,0), (0,0,1)),pbc=[True,True,False])
    slab.center(axis=2, vacuum=vac)
    calc = GPAW(nbands=-20,h=h, kpts=(k, k, 1), parallel={'domain':8}, txt='Bi111-da-k-'+var+"-"+str(num)+'.txt', xc='vdW-DF')
    slab.set_calculator(calc)
    e = slab.get_potential_energy()
    calc.write('Bi111-da-k-'+var+"-"+str(num)+'.txt')
    return e

name = 'Bi111-2-da-k'

'''f = paropen(name + '-da.dat', 'w')
print 'Energy with respect to da parameter'
for da in [4.460,4.465,4.470]:
   k = 2
   h = 0.18
   dl = 1.712
   vac = 8
   e = energy(k, h, dl, da, vac, 'da', da)
   print >> f, da, e'''

g = paropen(name + '-k.dat', 'w')
print 'Energy with respect to k value'
for k in [8,6,4,2]:
   h = 0.18
   da = 4.48
   dl = 1.712
   vac = 8
   e = energy(k, h, dl, da, vac, 'k', k)
   print >> g, k, e


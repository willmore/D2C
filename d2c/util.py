

"""
Taken from:
http://code.activestate.com/recipes/577105-synchronization-decorator-for-class-methods/

Defines a decorator, synchronous, that allows calls to methods
of a class to be synchronized using an instance based lock.
tlockname must refer to an instance variable that is some
kind of lock with methods acquire and release.  For thread safety this
would normally be a threading.RLock.

"""

from functools import wraps

def synchronous( tlockname ):
    """A decorator to place an instance based lock around a method """

    def _synched(func):
        @wraps(func)
        def _synchronizer(self,*args, **kwargs):
            tlock = getattr(self, tlockname)
            tlock.acquire()
            try:
                return func(self, *args, **kwargs)
            finally:
                tlock.release()
        return _synchronizer
    return _synched
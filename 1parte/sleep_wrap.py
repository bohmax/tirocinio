from ctypes import *
import sys


class Timespec(Structure):
    """ timespec struct for nanosleep, see:
      http://linux.die.net/man/2/nanosleep """
    _fields_ = [('tv_sec', c_long),
                ('tv_nsec', c_long)]


class Sleep:

    def __init__(self):
        path = 'libc.so.6'
        if sys.platform == 'darwin':
            path = '/usr/lib/libc.dylib'
        self.libc = CDLL(path)
        self.libc.usleep.argtypes = [c_uint32]
        self.libc.nanosleep.argtypes = [POINTER(Timespec),
                                        POINTER(Timespec)]

    """
    il Thread dorme per time millisecondi
    @:param time: deve essere compreso tra 0 1000000
    """
    def usleep(self, time):
        self.libc.usleep(c_uint32(time))

    """
    il Thread dorme per time nanosecondi
    @:param nsec1:  nanosecondi compresi tra 0 e 999999999 specifica per quanto tempo il thread deve dormire
    """
    def nsleep(self, nsec1):
        timespec = Timespec()
        timespec.tv_sec = c_long(0)
        timespec.tv_nsec = c_long(nsec1)
        self.libc.nanosleep(pointer(timespec), None)


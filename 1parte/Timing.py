import time
from ctypes import *
import sys
from decimal import Decimal

usec = 1000000
nanosec = 1000000000


class Timespec(Structure):
    """ timespec struct for nanosleep, see:
      http://linux.die.net/man/2/nanosleep """
    _fields_ = [('tv_sec', c_long),
                ('tv_nsec', c_long)]


class Timing:

    windows = False

    def __init__(self):
        if sys.platform != "win32":
            if sys.platform == 'darwin':
                self.libc = self.libc = CDLL('/usr/lib/libc.dylib')
            elif sys.platform == 'linux':
                self.libc = self.libc = CDLL('libc.so.6')
            self.libc.usleep.argtypes = [c_uint32]
            self.libc.nanosleep.argtypes = [POINTER(Timespec),
                                            POINTER(Timespec)]
        else:
            self.windows = True

    def delay_calculator(self, current_pkt_time, first_pkt_time, start_calculation_time):
        ideale = current_pkt_time - first_pkt_time
        attuale = time.time() - start_calculation_time
        return ideale - Decimal(attuale)

    """
    il Thread dorme per time millisecondi
    @:param time: deve essere compreso tra 0 1000000
    """
    def usleep(self, temp):
        if not self.windows:
            self.libc.usleep(c_uint32(temp*usec))
        else:
            self.__winsleep(time)

    """
    il Thread dorme per time nanosecondi
    @:param nsec1:  nanosecondi compresi tra 0 e 999999999 specifica per quanto tempo il thread deve dormire
    """
    def nsleep(self, temp):
        if not self.windows:
            timespec = Timespec()
            timespec.tv_sec = c_long(0)
            timespec.tv_nsec = c_long(temp*nanosec)
            self.libc.nanosleep(pointer(timespec), None)
        else:
            self.__winsleep(temp)

    """
    Windows non ha sleep, simulo solo una nsleep in maniera brutale
    """
    def __winsleep(self, time_to_wait):
        target_time = time.time() + float(time_to_wait)
        while time.time() < target_time:
            pass



import sys
import os
import time
import multiprocessing

"""
argv[1] deve contenere il path del file pcap che dovra essere salvato
argv[2] l'interfaccia da ascoltare
argv[2] deve contenere il path del file pcap che dovra essere salvato
argv[3] l'interfaccia da ascoltare
"""


def call(string):
    print(string)
    os.system(string)


if __name__ == '__main__':
    parameters = ['python3 SniffAndCreatePcap.py "' + sys.argv[3] + '" ' + sys.argv[4],
                  'python3 CreateRTPstream.py ' + sys.argv[1] + ' ' + sys.argv[2] ]

    with multiprocessing.Pool() as pool:
        pool.map(call, parameters)

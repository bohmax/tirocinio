import sys
import os
import time
import multiprocessing

"""
argv[1] deve contenere il path del file video da inviare
argv[2] deve contenere il path del file pcap che dovra essere salvato
argv[3] l'interfaccia da ascoltare
"""


def call(string):
    print(string)
    os.system(string)


if __name__ == '__main__':
    parameters = ['python3 getPcap.py "' + sys.argv[2] + '" "' + sys.argv[3] + '"',
                  'python3 sendToVLC.py "' + sys.argv[1] + '"']

    #for i in processes:
    #    i.start()
    #    time.sleep(1)
    with multiprocessing.Pool() as pool:
        pool.map(call, parameters)

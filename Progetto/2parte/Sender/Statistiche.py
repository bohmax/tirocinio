import socket
from _queue import Empty
from ctypes import *

HOST = '127.0.0.1'
PORT = 9998


class Stat(Structure):  # Struttura che deve essere identica alla struttura send_stat definita in struct.h
    _fields_ = [("perdita", c_uint16),
                ("lunghezza", c_uint16),
                ("delay", c_int),
                ("ordine", c_uint16)]


def stat(args):
    queue = args[0]
    num_lst = args[1]  # per sapere il numero di sender
    esci = args[2]
    dim = sizeof(Stat)
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, PORT))
        s.listen()
        conn, addr = s.accept()
        with conn:
            print('Connected by', addr)
            queue.put('conn')
            while not esci:
                try:
                    data = conn.recv(dim*num_lst, socket.MSG_WAITALL)  # aspetto che vengano ricevuti tutti i byte dichiarati non solo il massimo
                except KeyboardInterrupt:
                    break
                if not data:
                    break
                for i in range(num_lst):
                    test = data[i*dim:(i+1)*dim]
                    stat_ = Stat.from_buffer_copy(test)
                    print("Received perd=%d, lungh=%d, delay=%d, ord=%d" % (stat_.perdita, stat_.lunghezza, stat_.delay, stat_.ordine))
        print("Esco dalle statistiche")
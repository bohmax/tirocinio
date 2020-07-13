import csv
import socket
from Scheduler_controller import SchedulerController
from ctypes import *
from datetime import datetime


class Stat(Structure):  # Struttura che deve essere identica alla struttura send_stat definita in struct.h
    _fields_ = [("perdita", c_uint16),
                ("lunghezza", c_uint16),
                ("delay", c_double),
                ("jitter", c_double),
                ("ordine", c_uint16),
                ("num_of_pkt", c_int)]


def stat(args):
    data = datetime.now()
    path = '/Users/maxuel/PycharmProjects/tirocinio/Progetto/2parte/Sender/statistics/stat/' + str(data) + '.csv'
    queue = args[0]
    ip = args[1]
    port = args[2]
    num_lst = args[3]  # per sapere il numero di sender
    statistiche = []
    esci = False
    dim = sizeof(Stat)
    controller = SchedulerController()
    with open(path, 'w+') as f:
        writer = csv.writer(f)
        writer.writerow(["#Perdite", "Lunghezza perdite", "Delay", "Jitter", "Fuori ordine", "Number of packets"])
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.bind((ip, port))
            s.listen()
            conn, addr = s.accept()
            with conn:
                print('Connected by', addr)
                queue.put((controller.probabilitaSulNumeroLink(), controller.probabilitaSulLinkInvio()))
                while not esci:
                    try:
                        data = conn.recv(dim*num_lst, socket.MSG_WAITALL)  # aspetto che vengano ricevuti tutti i byte dichiarati non solo il massimo
                        if not data:
                            break
                        for i in range(num_lst):
                            test = data[i*dim:(i+1)*dim]
                            stat_ = Stat.from_buffer_copy(test)
                            statistiche.append((stat_.perdita/stat_.num_of_pkt, stat_.delay))
                            writer.writerow([stat_.perdita, stat_.lunghezza, stat_.delay, stat_.jitter, stat_.ordine, stat_.num_of_pkt])
                            #print("Received perd=%d, lungh=%d, delay=%d, ord=%d" % (stat_.perdita, stat_.lunghezza, stat_.delay, stat_.ordine))
                        if num_lst == 3:
                            loss1, delay1 = statistiche[0]
                            loss2, delay2 = statistiche[1]
                            loss3, delay3 = statistiche[2]
                            controller.setChannels(loss1, loss2, loss3, delay1, delay2, delay3)
                            queue.put((controller.probabilitaSulNumeroLink(), controller.probabilitaSulLinkInvio()))
                        writer.writerow(["", "", "", "", "", ""])
                    except KeyboardInterrupt:
                        break
        print("Esce dalle statistiche")
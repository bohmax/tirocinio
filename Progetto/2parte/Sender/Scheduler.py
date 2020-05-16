from scapy.layers.inet import UDP, IP
from Operatore import Operatore
from Sender import Sender
import Image_Handler
import Statistiche
from scapy.all import *
from Timing import *
from multiprocessing import Process, Queue
import time

"""
argv[1] deve contenere il path del file pcap che dovra essere letto
argv[2] l'interfaccia su cui mandare i pacchetti
argv[3] Serve per specificare l'ip a cui spedire, windows se mi trovo sul mac a casa, cnr, e windows se devo spedire al mac
attualmente specifica l'ip
argv[4] porta dello stream rtp del file pcap
argv[5] numero di canali
argv[6] porta iniziale dei canali
argv[7] Specifica la porta in cui saranno ricevuti le statistiche
argv[8] Specifica il path in cui saranno salvati i GOP
argv[9] Specifica il path in cui saranno salvati i frame
argv[10] Specifica il parametro gamma per decidere se si entra nell'evento perdita pacchetti
argv[11] Specifica il parametro beta per decidere se si entra nell'evento perdita pacchetti
argv[12] Specifica il parametro gamma per decidere quanti pacchetti scartare nell'evento perdita pacchetti
argv[13] Specifica il parametro beta per decidere quanti pacchetti scartare nell'evento perdita pacchetto
"""

conf.use_pcap = True  # permette di usare subito un nuovo socket appena lo creo
#sender = []
#nomi_operatori = ['Vodafone', 'Tim','Wind']
nomi_operatori = ['Vodafone', "Tim", "Wind"]
#nomi_operatori = ['Vodafone']
ip = sys.argv[3]
if sys.argv[3] == 'cnr':
    ip = '146.48.55.216'
elif sys.argv[3] == 'mac':
    ip = '192.168.1.176'
elif sys.argv[3] == 'ub_s':
    ip = '192.168.1.176'
elif sys.argv[3] == 'win_s':
    ip = '192.168.1.176'
elif sys.argv[3] == 'myself':
    ip = '127.0.0.1'

porta_pcap = int(sys.argv[4])  # porta dei pacchetti rtp
num_porte = int(sys.argv[5])
porta_inoltro = int(sys.argv[6])
port_stat = int(sys.argv[7])
gop_dir = sys.argv[8]
img_dir = sys.argv[9]
gamma_evento = int(sys.argv[10])
beta_evento = int(sys.argv[11])
gamma_perdita = int(sys.argv[12])
beta_perdita = int(sys.argv[13])
delay = float(sys.argv[14])


def canale(queue, nome, gamma_e, beta_e, gamma_p, beta_p, delay, interface, ip, porta):
    operatore = Operatore(name=nome, gamma_e=gamma_e, beta_e=beta_e, gamma_p=gamma_p, beta_p=beta_p, delay=delay)
    sender = Sender(operatore, interface, ip, porta)
    while True:
        try:
            pkt, index = queue.get()
        except KeyboardInterrupt:
            break
        else:
            if pkt is None:
                break
        sender.send_setted(pkt, index)


if __name__ == "__main__":
    process_list = []
    queue_list = []
    sender = Sender(None, sys.argv[2], ip, 0)  # non si spedirà mai con questo sender, usato solo per settare un pacchetto
    for i in range(num_porte):
        if i >= len(nomi_operatori):
            i = 2
        queue_list.append(Queue())
        process_list.append(Process(target=canale, args=(queue_list[i], nomi_operatori[i], gamma_evento, beta_evento,
                                                         gamma_perdita, beta_perdita, delay,
                                                         sys.argv[2], ip, porta_inoltro)))
        process_list[i].start()
        porta_inoltro += 1

    queue_stat = Queue()
    queue_gop = Queue()
    esci = False
    #stat_process = Process(target=Statistiche.stat, args=((queue_stat, num_porte, port_stat), ))
    #image_process = Process(target=Image_Handler.analyzer, args=((queue_gop, sys.argv[1], porta_pcap, gop_dir, img_dir), ))
    #image_process.start()
    #queue.get(block=False)
    #stat_process.join()
    #stat_process.start()
    #try:
    #    queue_stat.get()
    #except KeyboardInterrupt:
    #    exit(-1)
    packets = rdpcap(sys.argv[1], 1)
    timer = Timing()
    start = time.time()
    start_time = 0
    pkt_start_time = packets[0].time
    numero_totale_pkt = 0
    try:
        for index, pkt in enumerate(PcapReader(sys.argv[1])):
            if IP in pkt and UDP in pkt and pkt[UDP].dport == porta_pcap:
                print(index)
                try:
                    if start_time == 0:
                        start_time = time.time()
                    #queue_gop.put(pkt)
                    pkt_time = pkt.time
                    pkt = sender.set_packet(pkt)
                    timer.nsleep(timer.delay_calculator(pkt_time, pkt_start_time, start_time)*0.85)  # in modo da svegliarsi un 15% prima e poter eseguire più invii senza problemi
                    for i in queue_list:
                        i.put((pkt, index))
                except KeyboardInterrupt:
                    numero_totale_pkt = index
                    break
    except KeyboardInterrupt:
        pass

    #arr = sender[0].getOperatore().getNotSent()
    esci = True
    for i in queue_list:
        i.put((None, -1))
    for i in process_list:
        i.join()
    #queue_gop.put(None)
    #image_process.join()
    #print(arr)
    #print('Numero di elementi non inviati ' + str(len(arr)) + ' su ' + str(numero_totale_pkt) + ' pacchetti')
    print("--- %s seconds ---" % (time.time() - start))

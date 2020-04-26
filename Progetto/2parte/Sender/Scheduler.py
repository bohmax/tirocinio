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
argv[4] numero di canali
argv[5] porta iniziale dei canali
argv[4] Specifica la probabilita con la quale saranno scartati i pacchetti
"""

conf.use_pcap = True  # permette di usare subito un nuovo socket appena lo creo
sender = []
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
prob = 0
num_porte = int(sys.argv[4])
porta = int(sys.argv[5])
gop_dir = sys.argv[6]
img_dir = sys.argv[7]


if __name__ == "__main__":
    for i in range(num_porte):
        if i >= len(nomi_operatori):
            i = 2
        operatore = Operatore(name=nomi_operatori[i], prob=prob, loss=0, delay=0)
        sender.append(Sender(operatore, sys.argv[2], ip, porta))
        porta += 1

    #queue = Queue()
    queue_gop = Queue()
    esci = False
    #stat_process = Process(target=Statistiche.stat, args=((queue, len(nomi_operatori), esci), ))
    image_process = Process(target=Image_Handler.analyzer, args=((queue_gop, gop_dir, img_dir),))
    image_process.start()
    #queue.get(block=False)
    #stat_process.join()
    #stat_process.start()
    #try:
    #    queue.get()
    #except KeyboardInterrupt:
    #    exit(-1)
    packets = rdpcap(sys.argv[1], 1)
    timer = Timing()
    start = time.time()
    start_time = 0
    pkt_start_time = packets[0].time
    numero_totale_pkt = 0
    for index, pkt in enumerate(PcapReader(sys.argv[1])):
        if IP in pkt and UDP in pkt and pkt[UDP].dport == 5000:
            print(index)
            try:
                if start_time == 0:
                    start_time = time.time()
                queue_gop.put(pkt)
                timer.nsleep(timer.delay_calculator(pkt.time, pkt_start_time, start_time)*0.85)  # in modo da svegliarsi un 15% prima e poter eseguire più invii senza problemi
                for i in sender:
                    i.send(pkt, index)
            except KeyboardInterrupt:
                numero_totale_pkt = index
                break

    #arr = sender[0].getOperatore().getNotSent()
    esci = True
    queue_gop.put(None)
    image_process.join()
    #print(arr)
    #print('Numero di elementi non inviati ' + str(len(arr)) + ' su ' + str(numero_totale_pkt) + ' pacchetti')
    print("--- %s seconds ---" % (time.time() - start))

from scapy.layers.inet import UDP, IP
from scapy.layers.rtp import RTP
from Operatore import Operatore
from Sender import Sender
import Image_Handler
import Statistiche
from scapy.all import *
from Timing import *
from multiprocessing import Process, Queue
import time
import signal
signal.signal(signal.SIGINT, signal.default_int_handler)

"""
argv[1] deve contenere il path del file pcap che dovra essere letto
argv[2] l'interfaccia su cui mandare i pacchetti
argv[3] Serve per specificare l'ip in cui spedire i pacchetti
attualmente specifica l'ip
argv[4] numero di canali
argv[5] la prima porta di un canale, se ci sono più porta i canali saranno numerati come (argv[5], argv[5]+1, etc..)
argv[6] Specifica la porta in cui saranno ricevute le statistiche
argv[7] Specifica il path in cui saranno salvati i GOP
argv[8] Specifica il path in cui saranno salvati i frame
argv[9] Specifica se usare i valori gamma, 1 per usarli, 0 altrimenti
argv[10] Specifica il parametro gamma per decidere se si entra nell'evento perdita pacchetti
argv[11] Specifica il parametro beta per decidere se si entra nell'evento perdita pacchetti
argv[12] Specifica il parametro gamma per decidere quanti pacchetti scartare nell'evento perdita pacchetti
argv[13] Specifica il parametro beta per decidere quanti pacchetti scartare nell'evento perdita pacchetto
argv[14] Specifica il parametro gamma per decidere la durata del delay
argv[15] Specifica il parametro beta per decidere la durata del delay
"""

conf.use_pcap = True  # permette di usare subito un nuovo socket appena lo creo
nomi_operatori = ['Vodafone', "Tim", "Wind"]
pcap_path = sys.argv[1]
interface = sys.argv[2]
ip_receiver = sys.argv[3]
num_porte = int(sys.argv[4])
porta_inoltro = int(sys.argv[5])
port_stat = int(sys.argv[6])
gop_dir = sys.argv[7]
img_dir = sys.argv[8]
simula = int(sys.argv[9])
gamma_evento = float(sys.argv[10])
beta_evento = float(sys.argv[11])
gamma_perdita = float(sys.argv[12])
beta_perdita = float(sys.argv[13])
gamma_delay = float(sys.argv[14])
beta_delay = float(sys.argv[15])


def canale(queue, nome, interface, ip, porta, simulato, gamma_e, beta_e, gamma_p, beta_p, gamma_d, beta_d):
    operatore = Operatore(name=nome, simulate=simulato, gamma_e=gamma_e, beta_e=beta_e, gamma_p=gamma_p, beta_p=beta_p,
                          gamma_d=gamma_d, beta_d=beta_d)
    sender = Sender(operatore, interface, ip, porta)
    while True:
        try:
            pkt, index = queue.get()
            if pkt is None:
                break
            sender.send_setted(pkt, index)
        except KeyboardInterrupt:
            break


if __name__ == "__main__":
    process_list = []
    queue_list = []
    simulate = False
    if simula > 0:
        simulate = True
    sender = Sender(None, interface, ip_receiver, 0)  # non si spedirà mai con questo sender, usato solo per settare un pacchetto
    for i in range(num_porte):
        if i >= len(nomi_operatori):
            i = 2
        queue_list.append(Queue())
        process_list.append(Process(target=canale, args=(queue_list[i], nomi_operatori[i], interface, ip_receiver, porta_inoltro,
                                                         simulate, gamma_evento, beta_evento, gamma_perdita, beta_perdita,
                                                         gamma_delay, beta_delay)))
        process_list[i].start()
        porta_inoltro += 1

    queue_stat = Queue()
    queue_gop = Queue()
    esci = False
    stat_process = Process(target=Statistiche.stat, args=((queue_stat, sender.get_srcIP(), port_stat, num_porte), ))
    #image_process = Process(target=Image_Handler.analyzer, args=((queue_gop, pcap_path, gop_dir, img_dir), )) # da usare
    #image_process.start() # da usare
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
    bind_layers(UDP, RTP)
    try:
        for index, pkt in enumerate(PcapReader(pcap_path)):
            if IP in pkt and RTP in pkt:
                numero_totale_pkt = index
                if start_time == 0:  # per avere uno start time più fedele possibile
                    start_time = time.time()
                pkt_time = pkt.time
                pkt = sender.set_packet(pkt)
                timer.nsleep(timer.delay_calculator(pkt_time, pkt_start_time, start_time)*0.85)  # in modo da svegliarsi un 15% prima e poter eseguire più invii senza problemi
                for i in queue_list:
                    i.put((pkt, index))
    except KeyboardInterrupt:
        pass

    #arr = sender[0].getOperatore().getNotSent()
    esci = True
    for i in queue_list:
        i.put((None, -1))
    # queue_gop.put(None) # da usare
    for i in process_list:
        i.join()
    #image_process.join() # da usare
    #print(arr)
    #print('Numero di elementi non inviati ' + str(len(arr)) + ' su ' + str(numero_totale_pkt) + ' pacchetti')
    print("--- %s seconds ---" % (time.time() - start))

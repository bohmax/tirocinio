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
pcap_path, interface, ip_receiver, gop_dir, img_dir = "", "", "", "", ""
num_porte, porta_inoltro, port_stat, simula = 0, 0, 0, 0
alpha_evento, scale_evento, alpha_perdita, scale_perdita, alpha_delay, scale_delay = [], [], [], [], [], []


def inizializza():
    global pcap_path, interface, ip_receiver, gop_dir, img_dir, num_porte, porta_inoltro, port_stat, simula
    global alpha_evento, scale_evento, alpha_perdita, scale_perdita, alpha_delay, scale_delay
    pcap_path = sys.argv[1]
    interface = sys.argv[2]
    ip_receiver = sys.argv[3]
    num_porte = int(sys.argv[4])
    porta_inoltro = int(sys.argv[5])
    port_stat = int(sys.argv[6])
    gop_dir = sys.argv[7]
    img_dir = sys.argv[8]
    simula = int(sys.argv[9])
    for j in range(num_porte):
        alpha_evento.append(float(sys.argv[10]))
        scale_evento.append(float(sys.argv[11]))
        alpha_perdita.append(float(sys.argv[12]))
        scale_perdita.append(float(sys.argv[13]))
        alpha_delay.append(float(sys.argv[14]))
        scale_delay.append(float(sys.argv[15]))


def canale(queue, nome, interface, ip, porta, simulato, alpha_e, scale_e, alpha_p, scale_p, alpha_d, scale_d):
    operatore = Operatore(name=nome, simulate=simulato, alpha_e=alpha_e, scale_e=scale_e, alpha_p=alpha_p,
                          scale_p=scale_p, alpha_d=alpha_d, scale_d=scale_d)
    sender = Sender(operatore, interface, ip, porta)
    while True:
        try:
            pkt, index = queue.get()
            if pkt is None:
                break
            sender.send_setted(pkt, index)
        except KeyboardInterrupt:
            break
    queue.put(sender.getOperatore().getNotSent())


if __name__ == "__main__":
    process_list = []
    queue_list = []
    simulate = False
    if simula > 0:
        simulate = True
    sender = Sender(None, interface, ip_receiver, 0)  # non si spedirà mai con questo sender, usato solo per settare un pacchetto
    for i in range(num_porte):
        if i >= len(nomi_operatori): # si potrebbe anche eliminare l'arrai nomi operatori
            name = nomi_operatori[2]
        else:
            name = nomi_operatori[i]
        queue_list.append(Queue())
        process_list.append(Process(target=canale, args=(queue_list[i], name, interface, ip_receiver, porta_inoltro,
                                                         simulate, alpha_evento[i], scale_evento[i], alpha_perdita[i],
                                                         scale_perdita[i], alpha_delay[i], scale_delay[i])))
        process_list[i].start()
        porta_inoltro += 1

    queue_stat = Queue()
    queue_gop = Queue()
    esci = False
    stat_process = Process(target=Statistiche.stat, args=((queue_stat, sender.get_srcIP(), port_stat, num_porte), ))
    #image_process = Process(target=Image_Handler.analyzer, args=((queue_gop, pcap_path, gop_dir, img_dir), )) # da usare
    #image_process.start() # da usare
    stat_process.start()
    try:
        queue_stat.get()
    except KeyboardInterrupt:
        exit(-1)
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
    arr = []
    esci = True
    for i in queue_list:
        i.put((None, -1))
        time.sleep(1)
    #queue_gop.put(None) # da usare
    ''' per vedere se ci sono pacchetti mai spediti
    for i in process_list:
        i.join()

    for i in queue_list:
        arr.append(i.get())
    new = []
    for i in arr[0]:  # el per elemento del primo array
        conta = 0
        for x in range(1, 3):  # scorri le altre liste
            for val in arr[x]:
                if i >= val:
                    if i == val:
                        conta += 1
                else:
                    break
        if conta == 2:
            new.append(i)
    print(new)
    '''
    #image_process.join() # da usare
    #print(arr)
    #print('Numero di elementi non inviati ' + str(len(arr)) + ' su ' + str(numero_totale_pkt) + ' pacchetti')
    print("--- %s seconds ---" % (time.time() - start))

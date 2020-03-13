from scapy.layers.inet import UDP, IP
from Operatore import Operatore
from Sender import Sender
from scapy.all import *
from Timing import *

"""
argv[1] deve contenere il path del file pcap che dovra essere letto
argv[2] l'interfaccia su cui mandare i pacchetti
argv[3] Serve per specificare l'ip a cui spedire, windows se mi trovo sul mac a casa, cnr, e windows se devo spedire al mac
argv[4] Specifica la probabilita con la quale saranno scartati i pacchetti
"""

conf.use_pcap = True  # permette di usare subito un nuovo socket appena lo creo
sender = []
#nomi_operatori = ['Vodafone', 'Tim','Wind']
nomi_operatori = ['Vodafone']
ip = "192.168.1.216"
if sys.argv[3] == 'cnr':
    ip = '146.48.55.216'
elif sys.argv[3] == 'mac':
    ip = '192.168.1.125'
elif sys.argv[3] == 'myself':
    ip = '192.168.1.91'
prob = int(sys.argv[4])
porta = 4999


if __name__ == "__main__":
    timer = Timing()
    for i in nomi_operatori:
        porta += 1
        operatore = Operatore(name=i, prob=prob, loss=0, delay=0)
        sender.append(Sender(operatore, sys.argv[2], ip, porta))

    packets = rdpcap(sys.argv[1], 1)
    pkt_start_time = packets[0].time
    start = time.time()
    numero_totale_pkt = 0
    for index, pkt in enumerate(PcapReader(sys.argv[1])):
        if IP in pkt and UDP in pkt and pkt[UDP].dport == 5000:
            print(index)
            try:
                timer.nsleep(timer.delay_calculator(pkt.time, pkt_start_time, start))
                sender[0].send(pkt, index)
            except KeyboardInterrupt:
                numero_totale_pkt = index
                break

    arr = sender[0].getOperatore().getNotSent()
    print(arr)
    print('Numero di elementi non inviati ' + str(len(arr)) + ' su ' + str(numero_totale_pkt) + ' pacchetti')

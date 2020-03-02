from scapy.layers.inet import UDP, IP
from Operatore import Operatore
from Sender import Sender
from scapy.all import *
from Timing import *

"""
argv[1] deve contenere il path del file pcap che dovra essere letto
argv[2] l'interfaccia su cui mandare i pacchetti
argv[3] Serve per specificare l'ip da utilizzare, 0 se mi trovo a caso, qualsialsi altro valore se sono al cnr
"""

sender = []
#nomi_operatori = ['Vodafone', 'Tim','Wind']
nomi_operatori = ['Vodafone']
ip = "192.168.1.216"
if sys.argv[3] == 'cnr':
    ip = '146.48.55.216'
porta = 4999


if __name__ == "__main__":
    timer = Timing()
    for i in nomi_operatori:
        porta += 1
        operatore = Operatore(i, 2, 0, 1)
        sender.append(Sender(operatore, sys.argv[2], ip, porta))

    packets = rdpcap(sys.argv[1], 1)
    pkt_start_time = packets[0].time
    start = time.time()
    for index, pkt in enumerate(PcapReader(sys.argv[1])):
        if IP in pkt and UDP in pkt: #and pkt["UDP"].dport == 5000:
            timer.nsleep(timer.delay_calculator(pkt.time, pkt_start_time, start))
            sender[0].send(pkt, index)

    arr = sender[0].getOperatore().getNotSent()
    print(arr)
    print('lenghezza elementi non inviati ' + str(len(arr)))

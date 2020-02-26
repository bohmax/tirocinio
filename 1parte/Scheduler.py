from scapy.layers.inet import UDP, IP
from Operatore import Operatore
from Sender import Sender
from scapy.all import *
from sleep_wrap import *

"""
argv[1] deve contenere il path del file pcap che dovra essere letto
argv[2] l'interfaccia su cui mandare i pacchetti
"""

sender = []
#nomi_operatori = ['Vodafone', 'Tim','Wind']
nomi_operatori = ['Vodafone']
ip = "127.0.0.1"
porta = 5000


if __name__ == "__main__":
    timer = Timing()
    for i in nomi_operatori:
        porta += 1
        operatore = Operatore(i, 2, 0, 1)
        sender.append(Sender(operatore, sys.argv[2], ip, porta))

    packets = rdpcap(sys.argv[1])
    prec = packets[0]
    pkt_start_time = prec.time
    start = time.time()
    for index, pkt in enumerate(packets):
        if IP in pkt and UDP in pkt and pkt["UDP"].dport == 6000:
            timer.nsleep(timer.delay_calculator(pkt.time, prec.time, pkt_start_time, start))
            sender[0].send(pkt, index)
            prec = pkt

    arr = sender[0].getOperatore().getNotSent()
    print(arr)
    print(len(arr))

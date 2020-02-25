from scapy.layers.inet import UDP, IP

from Operatore import Operatore
from Sender import Sender
from scapy.all import *
from sleep_wrap import *

sender = []
#nomi_operatori = ['Vodafone', 'Tim','Wind']
nomi_operatori = ['Vodafone']
ip = "127.0.0.1"
porta = 5000
nanosec = 1000000000


def delay_calculator(current_pkt_time, last_pkt_time, first_pkt_time, start_calculation_time):
    delay_ideale = (current_pkt_time - last_pkt_time)
    ideale = current_pkt_time - first_pkt_time
    attuale = time.time() - start_calculation_time
    diff = (Decimal(attuale) - ideale)
    return (delay_ideale - diff)*nanosec


if __name__ == "__main__":
    sleep = Sleep()
    for i in nomi_operatori:
        porta += 1
        operatore = Operatore(i, 2, 0, 1)
        sender.append(Sender(operatore, ip, porta))

    packets = rdpcap("/Users/maxuel/Desktop/mandatoo.pcap")
    prec = packets[0]
    pkt_start_time = prec.time
    start = time.time()
    for index, pkt in enumerate(packets):
        if IP in pkt and UDP in pkt and pkt["UDP"].dport == 5000:
            sleep.nsleep(delay_calculator(pkt.time, prec.time, pkt_start_time, start))
            sender[0].send(pkt, index)
            prec = pkt

    arr = sender[0].getOperatore().getNotSent()
    print(arr)
    print(len(arr))

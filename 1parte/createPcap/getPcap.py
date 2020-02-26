from scapy.all import *
from scapy.layers.inet import UDP, IP

"""
argv[1] deve contenere il path del file pcap che dovra essere salvato
argv[2] l'interfaccia da ascoltare
"""
#IFACES.show() #metodo per ottenere tutti i nomi delle interfacce su windows disponibili
path = sys.argv[1]
interface = sys.argv[2]
s = conf.L2socket(iface=interface)
pkts = []


def pkt_callback(pkt):
    udp = pkt[UDP]
    udp.dport = 5000
    del pkt[IP].chksum
    del udp.chksum
    s.send(pkt)
    pkts.append(pkt)


if __name__ == '__main__':
    sniff(opened_socket=L2ListenTcpdump(iface=interface,
                                        filter="not icmp and udp and dst port 6000"), prn=pkt_callback, timeout=10)
    wrpcap(path, pkts)
    print(len(pkts))
    print('Fine Cattura')

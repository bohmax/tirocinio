from scapy.all import *
from scapy.layers.inet import UDP, IP

"""
argv[1] deve contenere il path del file pcap che dovra essere salvato
argv[2] l'interfaccia da ascoltare
"""
#IFACES.show() #metodo per ottenere tutti i nomi delle interfacce su windows disponibili
path = sys.argv[1]
interface = sys.argv[2]
conf.use_pcap = True
#s = conf.L2socket(iface=interface)


def pkt_callback(pkt):
    #udp = pkt[UDP]
    #udp.dport = 5000
    #del pkt[IP].chksum
    #del udp.chksum
    #s.send(pkt)
    #pkts.append(pkt)
    pass


if __name__ == '__main__':
    pkts = sniff(opened_socket=L2ListenTcpdump(iface=interface, filter="not icmp and udp and dst port 5000"),
                 store=True, timeout=10)
    wrpcap(path, pkts)
    print('Fine Cattura')

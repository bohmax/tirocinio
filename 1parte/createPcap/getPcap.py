from scapy.all import *
from scapy.layers.inet import UDP, IP

"""
argv[1] deve contenere il path del file pcap che dovra essere salvato
argv[2] l'interfaccia da ascoltare
"""
#IFACES.show() #to get all interface name in windows
path = sys.argv[1]
pkts = []
#path = '/Users/maxuel/Desktop/7seconds.pcap'

def pkt_callback(pkt):
    #udp = pkt[UDP]
    #udp.dport = 5000
    #del pkt[IP].chksum
    #del udp.chksum
    #sendp(pkt, iface="lo0")
    pkts.append(pkt)


sniff(iface=sys.argv[2], prn=pkt_callback, filter="not icmp and udp and dst port 6000", timeout=10)
wrpcap(path, pkts)

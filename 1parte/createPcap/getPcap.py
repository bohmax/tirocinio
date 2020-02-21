from scapy.all import *
from scapy.layers.inet import UDP, IP

pkts = []


def pkt_callback(pkt):
    udp = pkt[UDP]
    udp.dport = 5000
    del pkt[IP].chksum
    del udp.chksum
    sendp(pkt, iface="lo0")
    pkts.append(pkt)


sniff(iface="lo0", prn=pkt_callback, filter="not icmp and udp and dst port 6000", timeout=10)
wrpcap('/Users/maxuel/Desktop/7seconds.pcap', pkts)

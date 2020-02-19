from scapy.all import *


pkts = []


def pkt_callback(pkt):
    pkts.append(pkt)


sniff(iface="lo0", prn=pkt_callback, filter="not icmp and udp and dst port 5000", timeout=10)
wrpcap('/Users/maxuel/Desktop/7seconds.pcap', pkts)

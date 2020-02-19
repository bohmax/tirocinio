from scapy.all import *
from scapy.layers.inet import IP, UDP
from scapy.layers.rtp import RTP

new_dest = "146.48.55.216"

packets = rdpcap("/Users/maxuel/Desktop/7seconds.pcap")
for pkt in packets:
    if IP in pkt and UDP in pkt and pkt["UDP"].dport == 5000:
        pkt[IP].dst = new_dest
        del pkt[IP].chksum
        sendp(pkt, iface="lo0")


'''
Come modificare il pacchetto https://stackoverflow.com/questions/52111663/python-scapy-rtp-header-manipulation-how-can-i-decode-rtp-in-scapy
#### un-commment and change lines below to manipulate headers

            # packet[RTP].version = 0
            # packet[RTP].padding = 0
            # packet[RTP].extension = 0
            # packet[RTP].numsync = 0
            # packet[RTP].marker = 0
            # packet[RTP].payload_type = 0
            # packet[RTP].sequence = 0

            # packet[RTP].timestamp = 0

            packet[RTP].sourcesync = 0
            # packet[RTP].sync = 0

            ### Calculate UDP Checksum or they will now be wrong!

            #https://scapy.readthedocs.io/en/latest/functions.html

            checksum_scapy_original = packet[UDP].chksum

            # set up and calculate some stuff

            packet[UDP].chksum = None ## Need to set chksum to None before starting recalc
            packetchk = IP(raw(packet))  # Build packet (automatically done when sending)
            checksum_scapy = packet[UDP].chksum
            packet_raw = raw(packetchk)
            udp_raw = packet_raw[20:]
            # in4_chksum is used to automatically build a pseudo-header
            chksum = in4_chksum(socket.IPPROTO_UDP, packetchk[IP], udp_raw)  # For more infos, call "help(in4_chksum)"

            # Set the new checksum in the packet

            packet[UDP].chksum = checksum_scapy # <<<< Make sure you use the variable in checksum_scapy

            # needed below to test layers before printing newts/newsourcesync etc to console

            if pl[pkt].haslayer(UDP):
                newts = RTP(pl[pkt][UDP].payload).timestamp
                newsourcesync = RTP(pl[pkt][UDP].payload).sourcesync

            else:
                newts = 999
                newsourcesync = 999

            print("Changing packet {0} of {3} to new timestamp {1} SSRC {2} Old UDP chksum {4} >> New UDP chksum ???").format(pkt+1,newts,newsourcesync,numberofpckts,hex(checksum_scapy_original))

        else:
            print "Probably Not a UDP / RTP Packet# {0}".format(pkt)
'''
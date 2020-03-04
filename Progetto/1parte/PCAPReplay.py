from scapy.all import *
from scapy.layers.inet import IP, UDP
from scapy.layers.l2 import Ether
from Progetto.Timing import Timing

"""
argv[1] deve contenere il path del file pcap che dovra essere letto
argv[2] l'interfaccia su cui mandare i pacchetti
"""

arr = []
ip_cnr = "146.48.55.216"
localhost = "127.0.0.1"
ip_src = get_if_addr(sys.argv[2])
ip_home_dst = "192.168.1.216"
src_mac_address = get_if_hwaddr(sys.argv[2])
dst_mac_home = '88:88:88:88:87:88'
generic_mac = 'ff:ff:ff:ff:ff:ff'

timing = Timing()

packets = rdpcap(sys.argv[1], 1000)
for index, pkt in enumerate(packets):
    if IP in pkt and UDP in pkt and pkt["UDP"].dport == 5000:
        pkt[Ether].src = src_mac_address #se non lo modifico il router droppa l'inoltro
        pkt[IP].src = ip_src
        pkt[IP].dst = ip_home_dst
        del pkt[Ether].dst  # attenzione se si conosce la destinazione è meglio specificarla per evitare problemi di lentazza(es. destinazione non raggiungibile)
        del pkt[IP].chksum
        del pkt[UDP].chksum  # il checksum è calcolato anche per l'ip destinazione e ip sorgente, è necessario ricalcolarlo
        ''' 
        Attenzione se non si specifica il mac address src ci sono problemi di lentezza, se il mac address sorgente
        non è disponibile facendo una richiesta arp al router si incorrono a gravi problemi di lentezza
        pktm = Ether(src=src_mac_address)/pkt.getlayer(IP)
        pktm.time = pkt.time
        '''
        arr.append(pkt)
    else:
        print(index)

print("Inizio a inoltrare")
s = conf.L2socket(iface=sys.argv[2])
pkt_start_time = arr[0].time
start = time.time()
for i in arr:
    temp = timing.delay_calculator(i.time, pkt_start_time, start)
    timing.nsleep(temp)
    s.send(i)
fine = time.time()
difference = (fine - start)
s.close()
print('Tempo reale trasmissione ' + str(difference))
print('Tempo ideale ' + str(arr[len(arr)-1].time-arr[0].time))

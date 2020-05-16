from scapy.all import *
from scapy.layers.inet import IP, UDP
from scapy.layers.l2 import Ether, Loopback


class Sender:

    """
    @param operatore: canale su cui spedire il pacchetto
    @param ip: ip di destinazione su cui spedire i pacchetti
    @param port: porta su cui vengono spediti i pacchetti
    """

    def __init__(self, operatore, interface, ip, port):
        if LOOPBACK_NAME != interface:  # non è un interfaccia di loopback
            self._loopback = False
        else:
            self._loopback = True
        if not self._loopback or not sys.platform == 'linux':
            self._socket = conf.L2socket(iface=interface)
        else:
            conf.L3socket = L3RawSocket(iface=interface)  # https://scapy.readthedocs.io/en/latest/troubleshooting.html
            self._socket = conf.L3socket
        self._operatore = operatore
        self._mac_address = get_if_hwaddr(conf.iface)
        self._src_ip = get_if_addr(conf.iface)
        self._ip = ip
        self._port = port

    def set_packet(self, pkt):
        if not self._loopback:
            pkt[Ether].src = self._mac_address
            del pkt[Ether].dst
        elif not sys.platform == 'linux':
            pkt = Loopback()/pkt.getlayer(IP)
        else:
            pkt = pkt.getlayer(IP)
        pkt[IP].src = self._src_ip
        pkt[IP].dst = self._ip
        del pkt[IP].chksum
        del pkt[UDP].chksum
        return pkt

    def send_setted(self, pkt, indice):
        pkt[UDP].dport = self._port
        self._operatore.send(self._socket, pkt, indice)

    def send_unsetted(self, pkt, indice):
        pkt = self.set_packet(pkt)
        pkt[UDP].dport = self._port
        self._operatore.send(self._socket, pkt, indice)

    def setIP(self, dstip):
        self._ip = dstip

    def setPort(self, port):
        self._port = port

    def getIP(self):
        return self._ip

    def getPort(self):
        return self._port

    def getOperatore(self):
        return self._operatore

    def getMacAddress(self):
        return self._mac_address

    def __repr__(self):
        return "<Sender con ip: %s, porta %s>" % (self._ip, self._port)

    def __str__(self):
        return "Sender con ip: %s, porta %s" % (self._ip, self._port)

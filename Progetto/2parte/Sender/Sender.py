from scapy.all import *
from scapy.layers.inet import IP, UDP
from scapy.layers.l2 import Ether


class Sender:

    """
    @param operatore: canale su cui spedire il pacchetto
    @param ip: ip di destinazione su cui spedire i pacchetti
    @param port: porta su cui vengono spediti i pacchetti
    """

    def __init__(self, operatore, interface, ip, port):
        self._socket = conf.L2socket(iface=interface)
        self._operatore = operatore
        self._mac_address = get_if_hwaddr(conf.iface)
        self._src_ip = get_if_addr(conf.iface)
        self._ip = ip
        self._port = port

    def send(self, pkt, indice):
        pkt[Ether].src = self._mac_address
        pkt[IP].src = self._src_ip
        pkt[IP].dst = self._ip
        pkt[UDP].dport = self._port
        del pkt[Ether].dst
        del pkt[IP].chksum
        del pkt[UDP].chksum
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

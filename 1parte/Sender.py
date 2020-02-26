from scapy.all import *
from scapy.layers.inet import IP, UDP


class Sender:

    """
    @param operatore: canale su cui spedire il pacchetto
    @param ip: ip di destinazione su cui spedire i pacchetti
    @param port: porta su cui vengono spediti i pacchetti
    """

    def __init__(self, operatore, interface, ip, port):
        self._operatore = operatore
        self._ip = ip
        self._port = port
        self._socket = conf.L2socket(iface=interface)

    def send(self, pkt, indice):
        pkt[IP].dst = self._ip
        pkt[UDP].dport = self._port
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

    def __repr__(self):
        return "<Sender con ip: %s, porta %s e probabilità %s>" % (self._ip, self._port, self._prob)

    def __str__(self):
        return "Sender con ip: %s, porta %s e probabilità %s" % (self._ip, self._port, self._prob)

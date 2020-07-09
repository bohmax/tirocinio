from scapy.all import *
import socket
from scapy.layers.inet import IP, UDP
from scapy.layers.l2 import Ether, Loopback


class Operatore:

    """
    @param sender: canale su cui spedire il pacchetto
    @param ip: ip di destinazione su cui spedire i pacchetti
    @param port: porta su cui vengono spediti i pacchetti
    """

    def __init__(self, sender, ip, port, nome):
        if sender is not None:
            self._address = (ip, port)
            self._socket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
            self._sender = sender
            self._ip = ip
            self._port = port
            self._name = nome

    def send(self, pkt, indice):
        self._sender.send(self._socket, self._address, pkt, indice)

    def set_pkt(self, pkt):
        return bytes(pkt[UDP].payload)

    def setIP(self, dstip):
        self._ip = dstip

    def setPort(self, port):
        self._port = port

    def getAddress(self):
        return self._address

    def getSocket(self):
        return self._socket

    def getIP(self):
        return self._ip

    def getPort(self):
        return self._port

    def getSender(self):
        return self._sender

    def getName(self):
        return self._name

    def setName(self, name):
        self._name = name

    def __repr__(self):
        return "<Sender con ip: %s, porta %s>" % (self._ip, self._port)

    def __str__(self):
        return "Sender con ip: %s, porta %s" % (self._ip, self._port)

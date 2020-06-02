import math
import random
import struct
import time

from scapy.packet import Raw
from scipy.stats import gamma


class Operatore:

    #contiene gli indici dei pacchetti non inviati
    _pkt_losted = []

    """
    @param name: nome dell'operatore
    @param gamma_e: valore gamma per decidere se si deve entrare nell'evento
    @param beta_e: valore gamma per decidere se si deve entrare nell'evento
    @param gamma_p: valore gamma per decidere quanti pacchetti perdere
    @param beta_p: valore beta per decidere quanti pacchetti perdere
    @param loss: probabilità con la quale i pacchetti vengono droppati per motivi legati alla rete
    @param delay: ritardi tra un pacchetto e l'altro
    """
    def __init__(self, name, simulate, gamma_e, beta_e, gamma_p, beta_p, gamma_d, beta_d):
        self._name = name
        self._simulate = simulate
        self._gamma_e = gamma_e
        self._beta_e = beta_e
        self._gamma_p = gamma_p
        self._beta_p = beta_p
        self._gamma_d = gamma_d
        self._beta_d = beta_d
        self._delay = gamma.rvs(gamma_d, scale=beta_d, size=1)
        self._evento = gamma.rvs(gamma_e, scale=beta_e, size=1)
        self._perdita = gamma.rvs(gamma_p, scale=beta_p, size=1)
        self._delay_prec = 0
        self._counter = 0  # numero di paccheti da scartare
        self._indice = 0  # numero di pacchetti inviati
        self._delay_list = []  # lista in cui verrano inseriti i pacchetti da non spedire subito

    def send(self, socket, pkt, indice):
        if self._simulate:
            ret = self.send_simulate(socket, pkt, indice)
            if ret == 0:
                self._delay = gamma.rvs(self._gamma_d, scale=self._beta_d, size=1000)
                self._evento = gamma.rvs(self._gamma_e, scale=self._beta_e, size=1000)
                self._perdita = gamma.rvs(self._gamma_p, scale=self._beta_p, size=1000)
        else:  # ci sarà un ritardo e una perdita reale
            pkt = pkt/Raw(bytearray(struct.pack("d", time.time())))
            self.send_pkt(socket, pkt)

    def send_simulate(self, socket, pkt, indice):
        index = self._indice % 1000
        self.send_delayed(socket)  # spedisce i vecchi pacchetti
        if self._counter <= 0:  # mi sto chiedendo se non ho pacchetti da scartare
            rng = random.random()
            if rng > self._evento[index]:
                val = self._delay[index]  # calcola il delay
                delay = val - self._delay_prec
                if delay < 0:
                    delay = 0
                self._delay_prec = delay
                pkt = pkt / Raw(bytearray(struct.pack("d", time.time())))
                self._delay_list.append((pkt, time.time() + delay))
            else:  # entra nell'evento perdita
                self._counter = math.ceil(self._perdita[index]) - 1
                self._pkt_losted.append(indice)
            self._indice += 1
        else:  # se ho pacchetti da scartare a causa di un evento perdita
            self._counter -= 1
        return index

    def send_delayed(self, socket):
        for i in reversed(self._delay_list):
            el, times = i
            if time.time() > times:
                self.send_pkt(socket, el)
                self._delay_list.remove(i)

    def send_pkt(self, socket, pkt):
        try:
            socket.send(pkt)
        except KeyboardInterrupt:
            pass

    def setName(self, name):
        self._name = name

    def setDelay(self, delay):
        self._delay = delay

    def getName(self):
        return self._name

    #def getLoss(self):
    #    return self._loss

    def getDelay(self):
        return self._delay

    def getNotSent(self):
        return self._pkt_losted

    def __repr__(self):
        return "<Operatore con nome: %s, ha un delay di %s>" % (self._name, self._delay)

    def __str__(self):
        return "Nome: %s, ha un delay di %s" % (self._name, self._delay)

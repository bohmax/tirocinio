import math
import random
import time

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
    def __init__(self, name, gamma_e, beta_e, gamma_p, beta_p, delay):
        self._name = name
        #self._loss = loss
        self._gamme_e = gamma_e
        self._betta_e = beta_e
        self._gamme_p = gamma_p
        self._beta_p = beta_p
        self._delay = delay
        self._evento = gamma.rvs(gamma_e, scale=1 / beta_e, size=1000)
        self._perdita = gamma.rvs(gamma_p, scale=1 / beta_p, size=1000)
        self._counter = 0
        self._delay_list = []  # lista in cui verrano inseriti i pacchetti da non spedire subito

    def send(self, socket, pkt, indice):
        #index = indice % 1000
        rng = random.random()
        for i in reversed(self._delay_list):
            el, times = i
            if times > time.time():
                socket.send(el)
                self._delay_list.remove(i)
        #if self._counter > 0:
        #    self._counter -= 1
        #    return
        #if rng <= self._evento[index]:
        if rng <= 0.1: #delay
            self._delay_list.append((pkt, time.time()+random.uniform(0, self._delay)))
        else:
            socket.send(pkt)
        #else:
        #    self._counter = math.ceil(self._perdita[index])
        #    self._pkt_losted.append(indice)

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

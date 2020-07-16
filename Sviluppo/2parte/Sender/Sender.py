import math
import random
import struct
import queue
import time
from threading import Thread
from queue import Empty
from scipy.stats import gamma


class Sender:

    #contiene gli indici dei pacchetti non inviati
    _pkt_losted = []

    """
    @param simulate: True se si simula l'invio su una rete reale, False altimenti
    @param alpha_e: valore alpha per decidere se si deve entrare nell'evento
    @param scale_e: valore scale per decidere se si deve entrare nell'evento
    @param alpha_p: valore alpha per decidere quanti pacchetti perdere
    @param scale_p: valore scale per decidere quanti pacchetti perdere
    @param alpha_d: valore alpha per decidere di quanto far ritardare un pacchetto
    @param scale_d: valore scal per decidere di quanto far ritardare un pacchetto
    """
    def __init__(self, simulate, alpha_e, scale_e, alpha_p, scale_p, alpha_d, scale_d):
        self._simulate = simulate
        if simulate:
            self._alpha_e = alpha_e
            self._scale_e = scale_e
            self._alpha_p = alpha_p
            self._scale_p = scale_p
            self._alpha_d = alpha_d
            self._scale_d = scale_d
            self._delay = gamma.rvs(alpha_d, scale=scale_d, size=1)
            self._evento = gamma.rvs(alpha_e, scale=scale_e, size=1)
            self._perdita = gamma.rvs(alpha_p, scale=scale_p, size=1)
            self._delay_prec = 0
            self._counter = 0  # numero di paccheti da scartare
            self._indice = 0  # numero di pacchetti inviati
            self._delay_list = []  # lista in cui verrano inseriti i pacchetti da non spedire subito
            self._queue = queue.Queue()  # coda per inoltrare pacchetti al listThread
            self._listThread = Thread(target=self.send_delayed, args=(self._delay_list, self._queue))
            self._listThread.start()

    def send(self, socket, address, pkt, indice):
        if self._simulate:
            ret = self.send_simulate(socket, address, pkt, indice)
            if ret == 0:
                self._delay = gamma.rvs(self._alpha_d, scale=self._scale_d, size=1000)
                self._evento = gamma.rvs(self._alpha_e, scale=self._scale_e, size=1000)
                self._perdita = gamma.rvs(self._alpha_p, scale=self._scale_p, size=1000)
        else:  # ci sarà un ritardo e una perdita reale
            pkt = pkt + (bytearray(struct.pack("d", time.time())))
            self.send_pkt(socket, address, pkt)

    def send_simulate(self, socket, address, pkt, indice):
        index = self._indice % 1000
        if self._counter <= 0:  # mi sto chiedendo se non ho pacchetti da scartare
            rng = random.random()
            if rng > self._evento[index]:
                val = self._delay[index]  # calcola il delay
                delay = val - self._delay_prec
                if delay < 0:
                    delay = 0
                self._delay_prec = delay
                pkt = pkt + (bytearray(struct.pack("d", time.time())))
                self._queue.put((True, socket, address, pkt, (time.time() + delay)))  # il primo parametro indica al Thread di continuare a ciclare
            else:  # entra nell'evento perdita
                self._counter = math.ceil(self._perdita[index]) - 1
                self._pkt_losted.append(indice)
            self._indice += 1
        else:  # se ho pacchetti da scartare a causa di un evento perdita
            self._counter -= 1
            self._pkt_losted.append(indice)
        return index

    def send_delayed(self, delay_list, queue):
        temp_min = float('inf')  # rappresenta il timestamp del pacchetto con meno delay, inizializzato con un grande va
        cicla = True
        while cicla:
            if not delay_list:  # La lista è vuota, si attende un pacchetto.
                cicla, temp_min = self.insertList(delay_list, queue, True, temp_min)
                if not cicla:
                    continue
            while time.time() <= temp_min:  # si attende finché non c'è almeno un pacchetto da spedire
                cicla, temp_min = self.insertList(delay_list, queue, False, temp_min)
                if not cicla:
                    return
            temp_min = float('inf')
            for i in reversed(self._delay_list):  # c'è almeno un pacchetto da spedire
                socket, addr, pkt, times = i
                if time.time() > times:
                    self.send_pkt(socket, addr, pkt)
                    self._delay_list.remove(i)
                else:  # può essere aggiornato il tempo minimio
                    if temp_min > times:
                        temp_min = times

    def insertList(self, list, queue, blocking, min_time):
        cicla = True
        try:
            cicla, *i = queue.get(block=blocking)
            if cicla:
                socket, addr, pkt, time = i
                if min_time > time:
                    min_time = time
                list.append((socket, addr, pkt, time))
        except Empty:
            pass
        return cicla, min_time

    def send_pkt(self, socket, addr, pkt):
        try:
            socket.sendto(pkt, addr)
        except KeyboardInterrupt:
            pass

    def setDelay(self, delay):
        self._delay = delay

    def getThr(self):
        return self._listThread

    def getQueue(self):
        return self._queue

    def getDelay(self):
        return self._delay

    def getNotSent(self):
        return self._pkt_losted

    def __repr__(self):
        return "<Operatore con nome: %s, ha un delay di %s>" % (self._delay)

    def __str__(self):
        return "Nome: %s, ha un delay di %s" % (self._delay)

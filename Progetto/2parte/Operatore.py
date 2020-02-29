import random

class Operatore:

    #contiene gli indici dei pacchetti non inviati
    _pkt_losted = []

    """
    @param name: nome dell'operatore
    @param prob: probabilità con la quale si decide se inviare o meno un pacchetto
    @param loss: probabilità con la quale i pacchetti vengono droppati per motivi legati alla rete
    @param delay: ritardi tra un pacchetto e l'altro
    """
    def __init__(self, name, prob, loss, delay):
        self._name = name
        self._loss = loss
        self._prob = prob
        self._delay = delay

    def send(self, socket, pkt, indice):
        if indice != 271: #or random.randint(1, 100) > self._prob
            socket.send(pkt)
        else:
            self._pkt_losted.append(indice)

    def setProb(self, prob):
        self._prob = prob

    def setName(self, name):
        self._name = name

    def setLoss(self, loss):
        self._loss = loss

    def setDelay(self, delay):
        self._delay = delay

    def getName(self):
        return self._name

    def getProb(self):
        return self._prob

    def getLoss(self):
        return self._loss

    def getDelay(self):
        return self._delay

    def getNotSent(self):
        return self._pkt_losted

    def __repr__(self):
        return "<Operatore con nome: %s, ha un loss di %s e un delay di %s>" % (self._name, self._loss, self._delay)

    def __str__(self):
        return "Nome: %s, ha un loss di %s e un delay di %s" % (self._name, self._loss, self._delay)

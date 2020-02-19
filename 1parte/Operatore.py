class Operatore:

    """
    @param name: nome dell'operatore
    @param port: porta su cui vengono spediti i pacchetti
    @param prob: probabilità con la quale si decide se inviare o meno un pacchetto
    @param loss: probabilità con la quale i pacchetti vengono droppati per motivi legati alla rete
    @param delay: ritardi tra un pacchetto e l'altro
    """
    def __init__(self, name, port, prob, loss, delay):
        self._name = name
        self._port = port
        self._prob = prob
        self._loss = loss
        self._delay = delay

    def setName(self, name):
        self._name = name

    def setProb(self, prob):
        self._prob = prob

    def setLoss(self, loss):
        self._loss = loss

    def setDelay(self, delay):
        self._delay = delay

    def getName(self):
        return self._name

    def getPort(self):
        return self._port

    def getProb(self):
        return self._prob

    def getLoss(self):
        return self._loss

    def getDelay(self):
        return self._delay



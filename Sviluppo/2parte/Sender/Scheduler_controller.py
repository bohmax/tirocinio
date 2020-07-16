import numpy as np
from scipy.optimize import minimize as Min
from random import choices
from collections import Counter


class SchedulerController:

    def __init__(self):
        # delay link 1
        self.d1 = 0.11
        # loss link 1
        self.pl1 = 2*10**-3
        # delay link 2
        self.d2 = 0.14
        # loss link 2
        self.pl2 = 3*10**-2
        # delay link 3
        self.d3 = 0.07
        # loss link 3
        self.pl3 = 9*10**-2
        self.calcolaPesi()  # calcolo intermedio
        self.probabilitaPerdita1Link()  # prob di scegliere un canale
        self.second_ch()  # calcolo intermedio
        #self.optimizedProb()
        self.probabilitaPerdita2Link()  # probabilità di scegliere 2 canali
        self.probabilitaPerdita3Link()  # probabilità di scegliere 3 canali

    def calcolaPesi(self):
        d1, d2, d3 = self.d1, self.d2, self.d3
        # calculate weight 1
        self.p1 = (d1**-1/(d1**-1+d2**-1+d3**-1))
        # calculate weight 2
        self.p2 = (d2**-1/(d1**-1+d2**-1+d3**-1))
        # calculate weight 3
        self.p3 = (d3**-1/(d1**-1+d2**-1+d3**-1))

    def second_ch(self):
        # 2 PATH
        p1, p2, p3 = self.p1, self.p2, self.p3
        self.p12 = p2*p1/(1 - p2) + p1*p2/(1 - p1)
        self.p13 = p3*p1/(1 - p3) + p1*p3/(1 - p1)
        self.p23 = p3*p2/(1 - p3) + p2*p3/(1 - p2)

    def optimizedProb(self):
        p12, p13, p23 = self.p12, self.p12, self.p23
        self.p1d = (p12 + p13)/((p12 + p13) + (p12 + p23) + (p13 + p23))
        self.p2d = (p12 + p23)/((p12 + p13) + (p12 + p23) + (p13 + p23))
        self.p3d = (p13 + p23)/((p12 + p13) + (p12 + p23) + (p13 + p23))

    def probabilitaPerdita1Link(self):
        # 1 PATH
        # resulting packet loss with monopath
        self.plP1 = self.p1 * self.pl1 + self.p2 * self.pl2 + self.p3 * self.pl3

    def probabilitaPerdita2Link(self):
        # resulting packet loss with doublepath
        self.plP2 = self.p12 * self.pl1 * self.pl2 + self.p13 * self.pl1 * self.pl3 + self.p23 * self.pl3 * self.pl2

    def probabilitaPerdita3Link(self):
        #1 PATH
        # resulting packet loss with triplepath
        self.plP3 = self.pl1 * self.pl2 * self.pl3

    def probabilitaSulNumeroLink(self):
        # multipath loss vector
        return np.array([self.plP1, self.plP2, self.plP3])

    def probabilitaSulLinkInvio(self):
        # multipath loss vector
        return np.array([self.p1, self.p2, self.p3, self.p12, self.p13, self.p23])

    def setChannels(self, pl1, pl2, pl3, d1, d2, d3):
        self.d1 = d1
        self.pl1 = pl1
        # delay link 2
        self.d2 = d2
        # loss link 2
        self.pl2 = pl2
        # delay link 3
        self.d3 = d3
        # loss link 3
        self.pl3 = pl3
        self.calcolaPesi()  # calcolo intermedio
        self.probabilitaPerdita1Link()  # prob di scegliere un canale
        self.second_ch()  # calcolo intermedio
        #self.optimizedProb()
        self.probabilitaPerdita2Link()  # probabilità di scegliere 2 canali
        self.probabilitaPerdita3Link()  # probabilità di scegliere 3 canali

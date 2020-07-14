import csv
import fileinput
import signal
from datetime import datetime
import cv2
from threading import Thread
from pyqtgraph.Qt import QtWidgets, QtCore
import pyqtgraph as pg
signal.signal(signal.SIGINT, signal.SIG_IGN)


class MyWidget(pg.GraphicsWindow):

    def __init__(self, parent=None):
        super().__init__(parent=parent)
        self.mainLayout = QtWidgets.QVBoxLayout()
        self.setLayout(self.mainLayout)

        self.timer = QtCore.QTimer(self)
        self.timer.setInterval(100)  # in milliseconds
        self.timer.timeout.connect(self.animate)
        self.timer.start()

        self.plotItem = self.addPlot(title="Qualità streaming")

        self.plotDataItem = self.plotItem.plot([], pen=None,
                                               symbolBrush=(255, 0, 0), symbolSize=5, symbolPen=None)
        self.plotItem.setRange(yRange=(-0.5, 61))
        self.ax = self.plotItem.getAxis('bottom')
        self.ay = self.plotItem.getAxis('left')
        self.ay.setLabel("PSNR [Db]")
        self.ax.setLabel("Frame")
        self.plotItem.setDownsampling(mode='peak')
        self.plotItem.setClipToView(True)
        #self.plotItem.setMouseEnabled(y=False)


    def animate(self):
        global max, x_vals, y_vals, move, xpos, last_max
        self.plotDataItem.setData(x_vals[:dim], y_vals[:dim])
        self.ax.setTicks([my_xticks[:dim]])
        if last_max != max:
            self.plotItem.setRange(xRange=[max-display_n_frame, max+0.5])
        else:
            last_max = max


display_n_frame = 10
data = datetime.now()
path = 'statistics/plot/' + str(data) + '.csv'
### START QtApp #####
app = QtWidgets.QApplication([])
# Set graphical window, its title and size
pg.setConfigOptions(antialias=False)  # True seems to work as well
win = MyWidget()
win.show()
win.resize(800, 600)
win.raise_()
x_vals = []
y_vals = []
my_xticks = []
dim, max, num_frame = 0, 1.0, 0  # dimensione array da plattare, max è il valore massimo da plottare e numero tatale dei frame attualmente inseriti
distance, xpos, diff = 0, 0, 0  # distanza dall'ultimo gop utilizzato, posizione in cui è stato sollevato il tasto destro del mouse
last_max = 0
move, moved, esci = True, False, False
last_image = ""  # ultima immagine utile, da confrontare in caso l'ultima immagina non sia decodificata
current_gop, last_frame = 0, 0  # ultimo gop e frame utilizzato per l'ultima immagine
not_in_current_gop = []  # vengono memorizzati qua i gop superiori per pater calcolare il psnr in seguito
with open(path, 'w+') as f:
    writer = csv.writer(f)
    writer.writerow(["#GOP_number", "frame_number_within_GOP", "PSNR", "distance_from_original_img", "losted packet"])

def sync_gop():
    global current_gop, not_in_current_gop, last_frame, last_image, last_frame
    cicla = True  # esci sse non c'è un messaggio di fine del gop corrente
    while not_in_current_gop and cicla:
        cicla = False
        for i in not_in_current_gop[:]:
            string, *other = i
            if string == "Fine":
                string, gop_num = i
                if gop_num == current_gop:
                    current_gop += 1
                    last_frame = 0
                    not_in_current_gop.remove(i)
                    cicla = True
            else:
                im1, im2, temp_gop, temp_frame, arr = i
                if temp_gop == current_gop and last_frame+1 == temp_frame:
                    last_frame = temp_frame
                    psnr_calc(im1, im2, temp_gop, str(temp_frame), arr)
                    not_in_current_gop.remove(i)
                    cicla = True


def psnr_calc(im1, im2, gop, frame, arr):
    global dim, max, num_frame, last_image, path, distance
    if im1 != im2:  # se il path è identico si sono avuti problemi nella decodifica
        last_image = im1
        distance = 0
    else:  # usa l'ultima immagine decodificata
        im1 = last_image
        distance += 1
    img1 = cv2.imread(im1)  # im1 è l'immagine lato receiver
    img2 = cv2.imread(im2)
    try:
        psnr = cv2.PSNR(img1, img2)
    except:
        print("Immagine per PSNR non trovata o si usa la stessa immagine")
        return
    if psnr > 60:
        psnr = 60
    num_frame += 1
    x_vals.append(num_frame)
    y_vals.append(psnr)
    titolo = 'G' + str(gop) + '#F' + frame
    my_xticks.append((max, titolo))
    dim += 1
    max = dim
    with open(path, 'a') as f:
        writer = csv.writer(f)
        writer.writerow([gop, frame, psnr, distance, arr])


def get_input():
    global esci, last_frame, last_image, current_gop, not_in_current_gop
    for line in fileinput.input():
        line = line.rstrip('\n')
        if line == "Esci":
            esci = True
            break
        else:
            line = line.split(" ")  # attenzione gli uri con spazi non funzioneranno
            if line[0] == "Fine":
                gop_num = int(line[1])
                if gop_num == current_gop:  # gop corrente finito sincronizzati con il gop più recente
                    current_gop += 1
                    last_frame = 0
                    sync_gop()
                else:  # il fine è di un gop successivo
                    not_in_current_gop.append((line[0], gop_num))
            else:
                temp_gop = int(line[2])
                temp_frame = line[3]
                if current_gop == temp_gop:
                    last_frame = int(temp_frame)
                    psnr_calc(line[0], line[1], temp_gop, temp_frame, line[4:])
                else:
                    not_in_current_gop.append((line[0], line[1], temp_gop, int(temp_frame), line[4:]))  # line[4] sono tutti gli indici da 4 fino alla fine
    app.quit()

th = Thread(target=get_input)
th.start()
app.exec_()
th.join()
import csv
import fileinput
import time
from datetime import datetime
import matplotlib
import cv2
import matplotlib.pyplot as plt
from threading import Thread
from matplotlib.animation import FuncAnimation

display_n_frame = 7
data = datetime.now()
path = 'statistics/plot/' + str(data) + '.csv'
matplotlib.use('Qt5Agg')
fig, ax = plt.subplots()
plt.ylabel("Quality Db")
plt.xlabel("Frame")
plt.title("Grafico qualità streaming")
plt.ion()
ln, = ax.plot([], [], marker="o")
x_vals = []
y_vals = []
my_xticks = []
plt.xticks(x_vals, my_xticks)
dim, max, num_frame = 0, 1.0, 0  # dimensione array da plattare, max è il valore massimo da plottare e numero tatale dei frame attualmente inseriti
distance = 0  # distanza dall'ultimo gop utilizzato
ax.set_xlim(-0.5, display_n_frame + 0.5)
ax.set_ylim(0, 61)
move, esci = True, False
last_image = ""  # ultima immagine utile, da confrontare in caso l'ultima immagina non sia decodificata
current_gop, last_frame = 0, 0  # ultimo gop e frame utilizzato per l'ultima immagine
not_in_current_gop = []  # vengono memorizzati qua i gop superiori per pater calcolare il psnr in seguito
with open(path, 'w+') as f:
    writer = csv.writer(f)
    writer.writerow(["#GOP_number", "frame_number_within_GOP", "PSNR", "distance_from_original_img"])


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
                im1, im2, temp_gop, temp_frame = i
                if temp_gop == current_gop and last_frame+1 == temp_frame:
                    last_frame = temp_frame
                    psnr_calc(im1, im2, temp_gop, str(temp_frame))
                    not_in_current_gop.remove(i)
                    cicla = True


def psnr_calc(im1, im2, gop, frame):
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
    my_xticks.append(titolo)
    dim += 1
    max = dim
    with open(path, 'a') as f:
        writer = csv.writer(f)
        writer.writerow([gop, frame, psnr, distance])


def mypause(interval):
    manager = plt._pylab_helpers.Gcf.get_active()
    if manager is not None:
        canvas = manager.canvas
        if canvas.figure.stale:
            canvas.draw_idle()
        #plt.show(block=False)
        canvas.start_event_loop(interval)
    else:
        time.sleep(interval)


def onclickrelease(event):
    global move
    move = True


def onclick(event):
    global move
    move = False


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
                    psnr_calc(line[0], line[1], temp_gop, temp_frame)
                else:
                    not_in_current_gop.append((line[0], line[1], temp_gop, int(temp_frame)))


def init():
    ax.set_xlim(-0.5, display_n_frame + 0.5)
    ax.set_ylim(0, 61)
    ln.set_data([], [])
    return ln,


def animate(i):
    global max, x_vals, y_vals
    ln.set_data(x_vals[:dim], y_vals[:dim])
    plt.xticks(x_vals[:dim], my_xticks[:dim])
    if move:
        ax.set_xlim(max - display_n_frame - 1.5, max + 0.5)
    return ln,


ani = FuncAnimation(fig, animate, init_func=init, interval=1000)
cid1 = fig.canvas.mpl_connect('button_press_event', onclick)
cid = fig.canvas.mpl_connect('button_release_event', onclickrelease)
th = Thread(target=get_input)
th.start()
plt.show(block=False)
while not esci:
    try:
        mypause(0.200)
    except KeyboardInterrupt:
        break
th.join()
fig.canvas.mpl_disconnect(cid)
fig.canvas.mpl_disconnect(cid1)
plt.close(fig=fig)


'''
    vecchio codice funzionante
    while True:
    for line in fileinput.input():
        y = input('inserisci y\n')
        x = input('inserisci x\n')
        x_vals.append(float(x))
        y_vals.append(float(y))
        dim += 1
        if float(x) > max:
            max = float(x)
'''
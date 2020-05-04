import fileinput
import time
import matplotlib
import cv2
import matplotlib.pyplot as plt
from threading import Thread
from matplotlib.animation import FuncAnimation

matplotlib.use('Qt5Agg')
fig, ax = plt.subplots()
plt.ylabel("Quality")
plt.xlabel("Frame")
plt.title("Grafico qualità streaming")
plt.ion()
ln, = ax.plot([], [], marker="o")
x_vals = []
y_vals = []
my_xticks = []
plt.xticks(x_vals, my_xticks)
dim, max = 0, 1.0  # dimensione array da plattare, max è il valore massimo da plottare
ax.set_xlim(-0.5, 1.5)
ax.set_ylim(-0.2, 1.2)
move, esci = True, False


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
    global dim, max, esci
    num_frame = 0
    for line in fileinput.input():
        line = line.rstrip('\n')
        if line == "Esci":
            esci = True
            break
        else:
            line = line.split(" ")  # attenzione gli uri con spazi non funzioneranno
            img1 = cv2.imread(line[0])
            img2 = cv2.imread(line[1])
            psnr = cv2.PSNR(img1, img2)
            num_frame += 1
            print(psnr)
            x_vals.append(num_frame)
            y_vals.append(psnr)
            size = len(line[1]-1)
            nframe = line[1][size-10:size-4].lstreap('0')  # numero del frame
            ngop = line[1][size - 17:size - 11].lstrea('0')  # numero del frame
            str = 'GOP ' + ngop + '#frame ' + nframe
            my_xticks.append(str)
            dim += 1
            max = dim


def init():
    ax.set_xlim(-0.5, 1.5)
    ax.set_ylim(-0.2, 1.2)
    ln.set_data([], [])
    return ln,


def animate(i):
    if move:
        ax.set_xlim(max - 1.5, max + 0.5)
    ln.set_data(x_vals[:dim], y_vals[:dim])
    return ln,


ani = FuncAnimation(fig, animate, init_func=init, interval=1000)
cid1 = fig.canvas.mpl_connect('button_press_event', onclick)
cid = fig.canvas.mpl_connect('button_release_event', onclickrelease)
th = Thread(target=get_input)
th.start()
plt.show(block=False)
while not esci:
    mypause(0.200)
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
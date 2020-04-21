import fileinput
import time
from threading import Thread
import matplotlib
import matplotlib.pyplot as plt
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


def prova():
    global dim, max, esci
    for line in fileinput.input():
        line = line.rstrip('\n')
        if line == "Esci":
            esci = True
            break
        else:
            line = line.split(" ")
            x_vals.append(float(line[1]))
            y_vals.append(float(line[0]))
            dim += 1
            if float(line[1]) > max:
                max = float(line[1])


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
th = Thread(target=prova)
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
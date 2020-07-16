import time
import vlc
import sys
from PyQt5.QtGui import QPalette, QColor
if sys.platform == "darwin":
        from PyQt5.QtWidgets import QMacCocoaViewContainer, QApplication, QVBoxLayout, QWidget, QMainWindow

path = '/Users/maxuel/Desktop/Tirocinio/Video/7secondi.mp4'


class Finestra(QMainWindow):
    def __init__(self, master=None):
        QMainWindow.__init__(self, master)
        self.title = 'Video'
        self.width = 640
        self.height = 480
        vlcInstance = vlc.Instance()
        self.player = vlcInstance.media_player_new()
        media = vlcInstance.media_new_path(path)
        self.player.set_media(media)
        self.initUI()

    def initUI(self):
        self.setWindowTitle(self.title)
        self.widget = QWidget(self)
        self.setCentralWidget(self.widget)

        self.videoframe = QMacCocoaViewContainer(0)
        self.palette = self.videoframe.palette()
        self.palette.setColor(QPalette.Window, QColor(0, 0, 0))
        self.videoframe.setPalette(self.palette)
        self.videoframe.setAutoFillBackground(True)

        self.vboxlayout = QVBoxLayout()
        self.vboxlayout.addWidget(self.videoframe)
        self.widget.setLayout(self.vboxlayout)
        self.player.set_nsobject(int(self.videoframe.winId()))
        self.play()

    def play(self):
        self.resize(self.width, self.height)
        self.show()
        self.player.play()

app = QApplication(sys.argv)
finestra = Finestra()

#media = vlcInstance.media_new_path()
#media1 = vlcInstance.media_new("/Users/maxuel/Desktop/Tirocinio/Video/MarcoGiordano.mp4")
#media1 = vlcInstance.media_new("rtp://127.0.0.1:5000")
#media1.add_option("sout=file/ts:/Users/maxuel/Desktop/tirocinio/Video/7seconds.mov")
#player.set_media('/Users/maxuel/Desktop/Tirocinio/Video/7secondi.mp4')
#per mac
#widget = QWidget(self)
#self.setCentralWidget(widget)

if finestra == None:
    print("trovato")

sys.exit(app.exec_())
#Ended = 6
#current_state = finestra.get_state()
#while current_state != Ended:
#  current_state = player.get_state()

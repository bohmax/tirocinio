import time
import vlc

path = '/Users/maxuel/Desktop/Tirocinio/Video/7secondi.mp4'

inst = vlc.Instance()
param = [
    path,
    "sout=#rtp{dst=127.0.0.1,port=6000,mux=ts}"
    ]
Media = inst.media_new(*param)
player = Media.player_new_from_media()
player.play()

time.sleep(10)

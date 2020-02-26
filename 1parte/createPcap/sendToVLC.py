import time
import os
import sys
if os.name == 'nt':
    os.add_dll_directory(r'C:\Program Files\VideoLAN\VLC')
import vlc

path = sys.argv[1]
inst = vlc.Instance()
param = [
    path,
    "sout=#transcode{vcodec=h264,acodec=mpga,ab=128,channels=2,samplerate=44100,scodec=none}:rtp{dst=127.0.0.1,port=6000,mux=ts}"
    ]
Media = inst.media_new(*param)
player = Media.player_new_from_media()
player.play()

time.sleep(10)

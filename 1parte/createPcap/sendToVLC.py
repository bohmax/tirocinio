import time
import os
import sys
#if os.name == 'nt':
#    os.add_dll_directory(r'C:\Program Files\VideoLAN\VLC')
import vlc
import gi
gi.require_version('Gst', '1.0')
gi.require_version("Gtk", "3.0")
from gi.repository import Gst, Gtk, GLib

"""
argv[1] deve contenere il path del file video da inviare
"""

#146.48.52.75
if __name__ == '__main__':
    Gst.init(None)
    pipeline = Gst.parse_launch('videotestsrc ! video/x-raw,framerate=20/1 ! videoscale ! videoconvert ! x264enc '
                                'tune=zerolatency bitrate=500 speed-preset=superfast ! rtph264pay ! '
                                'udpsink host=127.0.0.1 port=5000')
    ret = pipeline.set_state(Gst.State.PLAYING)
    if ret == Gst.StateChangeReturn.FAILURE:
        print("Unable to set pipeline to playing")
    else:
        time.sleep(10)
    pipeline.set_state(Gst.State.NULL)
    pipeline = None


"""
    time.sleep(0.5)
    print("Spedisco a VLC")
    path = sys.argv[1]
    inst = vlc.Instance()
    param = [
        path,
        "sout=#rtp{dst=146.48.55.217,port=5000,mux=ts}"
        ]
    Media = inst.media_new(*param)
    player = Media.player_new_from_media()
    player.play()

    time.sleep(6)
"""

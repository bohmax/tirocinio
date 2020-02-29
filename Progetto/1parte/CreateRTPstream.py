import gi
from scapy.all import *
gi.require_version('Gst', '1.0')
gi.require_version("Gtk", "3.0")
from gi.repository import Gst, Gtk, GLib

"""
argv[1] ip su cui dovrà essere spedito il video
argv[2] porta su cui sarà spedito il pacchetto
"""

ip = sys.argv[1]
port = sys.argv[2]


if __name__ == '__main__':
    Gst.init(None)
    pipeline = Gst.parse_launch('videotestsrc ! video/x-raw,framerate=20/1 ! videoscale ! videoconvert ! x264enc '
                                'tune=zerolatency bitrate=500 speed-preset=superfast ! rtph264pay ! '
                                'udpsink host=' + ip + ' port=' + port)
    time.sleep(1.5)  # per dare tempo alla sniffer di aprirsi
    ret = pipeline.set_state(Gst.State.PLAYING)
    if ret == Gst.StateChangeReturn.FAILURE:
        print("Unable to set pipeline to playing")
    time.sleep(10)
    pipeline.set_state(Gst.State.NULL)
    pipeline = None

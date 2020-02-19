import cv2

gst_str_rtp = "appsrc ! videoconvert ! x264enc tune=zerolatency bitrate=500 speed-preset=superfast ! rtph264pay ! udpsink host=127.0.0.1 port=5000"
fourcc = cv2.VideoWriter_fourcc(*'H264')
manda = "sout=#rtp{dst=127.0.0.1,port=5000,mux=ts}"
HOST = '127.0.0.1'
PORT = 5000
fps = 30.
frame_width = 1280
frame_height = 720
print(cv2.getBuildInformation())

class Streamto(object):
    def __init__(self):
        # Using OpenCV to capture from device 0. If you have trouble capturing
        # from a webcam, comment the line below out and use a video file
        # instead.
        self.video = cv2.VideoWriter(gst_str_rtp, 0, fps, (frame_width, frame_height), True)

    def __del__(self):
        self.video.release()

    def send(self, frame):
        self.video.write(frame)

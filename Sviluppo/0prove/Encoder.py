import cv2
import numpy

pathMarco = "/Users/maxuel/Desktop/Tirocinio/Video/MarcoGiordano.mp4"
pathCloack = "/Users/maxuel/Desktop/tirocinio/Video/7seconds.mov"
pathDesk = "/Users/maxuel/Desktop/"
length = 0


class VideoCamera(object):
  def __init__(self, path):
    # Using OpenCV to capture from device 0. If you have trouble capturing
    # from a webcam, comment the line below out and use a video file
    # instead.
    self.video = cv2.VideoCapture(path)
    self.length = int(self.video.get(cv2.CAP_PROP_FRAME_COUNT))
    # If you decide to use video.mp4, you must have this file in the folder
    # as the client.py.
    # self.video = cv2.VideoCapture('video.mp4')

  def __del__(self):
    self.video.release()

  def get_frame(self):
    success, frame = self.video.read()
    #data = frame.flatten().tostring()
    #ret, jpeg = cv2.imencode('.jpg', frame)
    #data = cv2.cvtColor(jpeg, cv2.COLOR_BGR2RGB)
    self.length -= 1
    return frame

  def create_frame(self, frame):
    ret, jpeg = cv2.imencode('.jpg', frame)
    self.length += 1
    return numpy.array(jpeg).tostring()

  def remaining_frame(self):
    return self.length

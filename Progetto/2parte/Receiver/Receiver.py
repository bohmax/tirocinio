import base64

import numpy as np
from scapy.all import *
from scapy.layers.inet import IP, UDP
from scapy.layers.rtp import RTP
import cv2.cv2 as cv2

"""
argv[1] deve contenere il path del file pcap che dovra essere letto
"""
os.environ["OPENCV_FFMPEG_CAPTURE_OPTIONS"] = "protocol_whitelist;file,rtp,udp"
path = sys.argv[2]
print(sys.argv[2])

if __name__ == "__main__":
    cap = cv2.VideoCapture(path)
    print('yey')
    success, image = cap.read()
    print('letto')
    count = 0
    while success:
        cv2.imwrite("/Users/maxuel/Desktop/files/frame%d.jpg" % count, image)  # save frame as JPEG file
        success, image = cap.read()
        print('Read a new frame: ', success)
        count += 1
    '''
    payloads = [bytearray()]
    bind_layers(UDP, RTP, dport=5000)
    for index, pkt in enumerate(PcapReader(sys.argv[1])):
        if IP in pkt and UDP in pkt and pkt[UDP].dport == 5000:
            payloads[len(payloads)-1] += pkt[Raw].load
            #pkt[Raw].show()
            if pkt[RTP].marker == 1:
                payloads.append(bytearray())
                break
    for i in payloads:
        print('gosh')
        jpg_original = base64.b64decode(i)
        jpg_as_np = np.frombuffer(jpg_original, dtype=np.uint8)
        img = cv2.imdecode(jpg_as_np, flags=1)
        cv2.imshow('jpg', img)
        cv2.waitKey(0)
        print('rip')
        #print(i)
    '''

'''
Come modificare il pacchetto https://stackoverflow.com/questions/52111663/python-scapy-rtp-header-manipulation-how-can-i-decode-rtp-in-scapy
#### un-commment and change lines below to manipulate headers

# packet[RTP].version = 0
# packet[RTP].padding = 0
# packet[RTP].extension = 0
# packet[RTP].numsync = 0
# packet[RTP].marker = 0
# packet[RTP].payload_type = 0
# packet[RTP].sequence = 0
# packet[RTP].timestamp = 0
'''
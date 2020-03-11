import base64
from imageio import imread
import matplotlib.pyplot as plt

import numpy as np
import cv2.cv2 as cv2
from scapy.all import *
from scapy.layers.inet import IP, UDP
from scapy.layers.rtp import RTP

"""
argv[1] deve contenere il path del file pcap che dovra essere letto
"""
os.environ["OPENCV_FFMPEG_CAPTURE_OPTIONS"] = "protocol_whitelist;file,rtp,udp"
#path = sys.argv[2]
#print(sys.argv[2])


def starter(to_add):
    for j in range(2):
        to_add += (0).to_bytes(1, byteorder=sys.byteorder)
    to_add += (1).to_bytes(1, byteorder=sys.byteorder)


if __name__ == "__main__":
    '''
    cap = cv2.VideoCapture(path)
    success, image = cap.read()
    count = 0
    while success:
        cv2.imwrite("/Users/maxuel/Desktop/files/frame%d.jpg" % count, image)  # save frame as JPEG file
        success, image = cap.read()
        print('Read a new frame: ', success)
        count += 1
    '''
    #payloads = [bytearray()]
    payload = bytearray()
    metadata = bytearray()
    bind_layers(UDP, RTP, dport=5000)
    num_frame = 0
    start_frame = 0
    lenght = 0
    #idr_nal = 0b00000000  # [ 3 NAL UNIT BITS | 5 FRAGMENT TYPE BITS] 8 bits
    inizialized = False
    for index, pkt in enumerate(PcapReader(sys.argv[1])):
        if IP in pkt and UDP in pkt and pkt[UDP].dport == 5000:
            # https://stackoverflow.com/questions/7665217/how-to-process-raw-udp-packets-so-that-they-can-be-decoded-by-a-decoder-filter-i
            #prendo H264 FRAGMENT

            data = pkt[Raw].load
            payload_type = pkt[RTP].payload_type
            lenght += len(data)
            #fragment_size = lenght - 2

            #FU - A - -HEADER
            forbidden = data[0] & 0x80 >> 7
            nri = data[0] & 0x60 >> 5
            fragment_type = data[0] & 0x1F  # spero che il valore sia 28

            #Nal header
            start_bit = data[1] & 0x80  # 128 se e' il primo pacchetto del frame 0 altrimenti
            end_bit = data[1] & 0x40  # 64 se e' l ultimo pacchetto 0 altrimenti
            reserved = data[1] & 0x20 >> 5
            nal_type = data[1] & 0x1F

            #payloads[len(payloads)-1] += pkt[Raw].load

            if fragment_type != 28 and not inizialized:
                starter(metadata)
                metadata += pkt[Raw].load  # SPS informaition && PPS information
                if fragment_type == 8:
                    metadata += payload
                    payload = metadata
                    inizialized = True
                    #f = open("/Users/maxuel/Desktop/hope", "wb")
                    #f.write(payload)
                    #break
            else:  # e' un frame video fragment_type = 28
                if start_bit == 128:
                    print('started ' + str(start_frame))
                    start_frame += 1
                    idr_nal = data[0] & 0xE0  # 3 NAL UNIT BITS
                    nal = idr_nal | nal_type
                    starter(payload)
                    payload += nal.to_bytes(1, byteorder=sys.byteorder)  # header per il frame
                payload += data[2:]  # rimuove gli header del payload e contiene solo i dati successivi
                if pkt[RTP].marker == 1:
                    num_frame += 1
                    if num_frame == 18 and inizialized:
                        f = open("/Users/maxuel/Desktop/hope", "wb")
                        f.write(metadata)
                        break

    '''
    print('gosh')
    jpg_original = base64.b64decode(payload)
    #invImg = cv2.bitwise_not(jpg_original)
    jpg_as_np = np.frombuffer(jpg_original, dtype=np.uint8)
    #invImg = cv2.bitwise_not(jpg_as_np)
    img_str = cv2.imencode('.jpg', jpg_as_np)[1]
    #print(img_str)
    #img = cv2.imdecode(jpg_as_np, flags=1)
    img2 = cv2.imdecode(img_str, cv2.IMREAD_UNCHANGED)
    cv2_img = cv2.cvtColor(img2, cv2.COLOR_RGB2BGR)
    print(img2)
    cv2.imwrite('/Users/maxuel/Desktop/img.jpg', img2)
    #cv2.imshow('jpg', img2)
    #cv2.waitKey(0)
    print('rip')
    #print(i)
    

    b64_bytes = base64.b64encode(payload)
    b64_string = b64_bytes.decode()

    # reconstruct image as an numpy array
    img = imread(io.BytesIO(base64.b64decode(payload)))

    # show image
    plt.figure()
    plt.imshow(img, cmap="gray")

    # finally convert RGB image to BGR for opencv
    # and save result
    cv2_img = cv2.cvtColor(img, cv2.COLOR_RGB2BGR)
    cv2.imwrite("/Users/maxuel/Desktop/img.jpg", cv2_img)
    plt.show()
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
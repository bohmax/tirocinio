import base64
from binascii import unhexlify

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
    bind_layers(UDP, RTP, dport=5000)
    idr_nal = 0b00000000  # [ 3 NAL UNIT BITS | 5 FRAGMENT TYPE BITS] 8 bits
    for index, pkt in enumerate(PcapReader(sys.argv[1])):
        if IP in pkt and UDP in pkt and pkt[UDP].dport == 5000:
            # https://stackoverflow.com/questions/7665217/how-to-process-raw-udp-packets-so-that-they-can-be-decoded-by-a-decoder-filter-i
            #prendo H264 FRAGMENT

            data = pkt[Raw].load
            fragment_type = data[0] & 0x1F  # spero che il valore sia 28
            nal_type = data[1] & 0x1F
            start_bit = data[1] & 0x80  # 128 se e' il primo pacchetto del frame 0 altrimenti
            end_bit = data[1] & 0x40  # 64 se e' l ultimo pacchetto 0 altrimenti
            #payloads[len(payloads)-1] += pkt[Raw].load

            if fragment_type == 7:  # non so ancora cosa voglia dire
                print('pacchetto numero ' + str(index))
            elif fragment_type == 28:  # e' un frame video
                if start_bit == 128:
                    idr_nal = data[0] & 0x7
                elif idr_nal != 0:
                    byte_idr_nal = ((idr_nal << 5) + fragment_type).to_bytes(1, byteorder=sys.byteorder)
                    payload[0:0] = byte_idr_nal  # fa una prepend
                    idr_nal = idr_nal >> 8
                    # ora devo scrivere questo valore in un buffer
            payload += data[2:]  # rimuove gli header del payload e contiene solo i dati successivi
            if pkt[RTP].marker == 1:
                #payloads.append(bytearray())
                nparr = np.frombuffer(payload, dtype="uint8")
                img_np = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
                print(img_np)
                break
    '''
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
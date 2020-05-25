from _queue import Empty
from multiprocessing import Process
from scapy.all import *
from scapy.layers.inet import UDP, IP
from scapy.layers.rtp import RTP

to_null = "&> /dev/null"


def starter(to_add):
    zero = (0).to_bytes(1, byteorder=sys.byteorder)
    uno = (1).to_bytes(1, byteorder=sys.byteorder)
    to_add += zero + zero + uno


def to_image(args):
    gop_path = args[0]
    img_path = args[1]
    with open(os.devnull, 'w') as fp:
        subprocess.Popen(["ffmpeg", "-i", gop_path, img_path], stdin=None, stdout=fp, stderr=fp)

def analyzer(args):
    queue = args[0]
    pcap_path = args[1]
    path_gop = args[2]
    path_img = args[3]
    num_frame = 0
    num_gop = 0
    payload = bytearray()
    metadata = bytearray()
    inizialized = False
    print(pcap_path)
    bind_layers(UDP, RTP)
    '''
    Devo iniziare a unificare quando ricevo pacchetti Header(nal type == 5)
    i pacchetti P sono con nal_type == 1
    Gli header sono preceduti da pacchetti SPS e PPS rispettivamente fragment_type == 7 fragment_type == 8
    il formato deve essere
    00 00 01 [SPS] 00 00 01 [PPS] 00 00 01 [DATA] in data bisogna togliere il nal header

    Inizio il mio algoritmo(venendo incontro in modo furbo al pcap di prova)
    scarto tutti i pacchetti finche non trovo un sps e pps
    cerco header e attacco idr nal e nal type, per i pacchetti header questo valore è sempre lo stesso
    inserisco poi il payload togliendo i primi due byte(contengono gli header)
    se incontro poi altri sps e pps li scarto(per lo stream in analisi sono sempre uguali)
    '''
    try:
        for pkt in PcapReader(pcap_path):
            try:
                val = queue.get(block=False)
            except Empty:
                pass
            else:
                if val == 'Esci':
                    break
            if IP in pkt and RTP in pkt:
                # https://stackoverflow.com/questions/7665217/how-to-process-raw-udp-packets-so-that-they-can-be-decoded-by-a-decoder-filter-i
                #prendo H264 FRAGMENT
                data = pkt[RTP].load

                #FU - A - -HEADER
                forbidden = data[0] & 0x80 >> 7
                nri = data[0] & 0x60 >> 5
                fragment_type = data[0] & 0x1F  # spero che il valore sia 28

                if not inizialized:
                    if fragment_type == 7:
                        starter(metadata)
                        metadata += pkt[Raw].load  # SPS informaition
                    if fragment_type == 8:
                        starter(metadata)
                        metadata += pkt[Raw].load  # PPS information
                        inizialized = True
                    continue

                #informazioni per lo streaming ottenuto, inizio concatenazione
                if fragment_type == 28:
                    #Nal header
                    start_bit = data[1] & 0x80  # 128 se e' il primo pacchetto del frame 0 altrimenti
                    end_bit = data[1] & 0x40  # 64 se e' l ultimo pacchetto 0 altrimenti
                    reserved = data[1] & 0x20 >> 5
                    nal_type = data[1] & 0x1F

                    if start_bit == 128:
                        if nal_type == 5:
                            if len(payload) != 0:
                                path = path_gop + 'gop-' + str(num_gop).zfill(6)
                                img_path = path_img + "frame-" + str(num_gop).zfill(6) + "-%06d.png"
                                f = open(path, "wb")
                                f.write(payload)
                                f.close()
                                num_gop += 1
                                payload = bytearray()
                                Process(target=to_image, args=((path, img_path),)).start()
                            print('start new gop ' + str(num_gop))
                            payload += metadata
                        idr_nal = data[0] & 0xE0  # 3 NAL UNIT BITS
                        nal = idr_nal | nal_type  # [ 3 NAL UNIT BITS | 5 FRAGMENT TYPE BITS] 8 bits
                        starter(payload)
                        payload += nal.to_bytes(1, byteorder=sys.byteorder)  # header per il frame
                    elif end_bit == 64:
                        num_frame += 1  # solo per dati statistici
                    payload += data[2:]  # rimuove gli header del payload e contiene solo i dati successivi
    except KeyboardInterrupt:
        pass

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
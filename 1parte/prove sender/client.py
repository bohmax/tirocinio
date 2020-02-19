from Stream_out import Streamto
from Encoder import VideoCamera

path = "/Users/maxuel/Desktop/Tirocinio/Video/7secondi.mp4"
stream = VideoCamera(path)
streamout = Streamto()
#client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
#serverAddressPort = ("127.0.0.1", 5000)
remaining = stream.remaining_frame()

while remaining > 0:
    try:
        print(remaining)
        frame = stream.get_frame()
        #composite_list = [line[x:x + 4096] for x in range(0, len(line), 4096)]
        #for el in composite_list:
        streamout.send(frame)
        remaining = stream.remaining_frame()
    #trovato = False
    #while not trovato:
    #    data = client_socket.recv(4096)
    #    print(data)
    #    frame = numpy.fromstring(data, dtype=numpy.uint8)
    #    frame = numpy.reshape(frame, (240, 320, 3))
        #decodificato = letto.decode('utf-8')
    #    decodificato = numpy.fromstring(letto, dtype='uint8')
        #split = decodificato.split('\n\n', 1)
        #file.write(split[0])
        #if len(split) == 2:
        #    file.write(split[1])
        #    trovato = True
    #toread -= 1
    except KeyboardInterrupt:
        break

print('finito client')

import socket
import numpy

serv = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
serv.bind(('127.0.0.1', 5000))
val = 0
while True:
    try:
        data, addr = serv.recvfrom(4096)
        print(val)
        val += 1
        frame = numpy.fromstring(data.decode('utf-8'), dtype=numpy.uint8)
        #frame = frame.reshape(480, 640, 3)
        #tobyte = stream.get_frame()
        #print(stream.remaining_frame())
        #print(tobyte)
        #client_socket.send(str(len(tobyte)).ljust(16))
        #client_socket.send(tobyte)
    except KeyboardInterrupt:
        break

print('finito server')
import socket
import struct
import time

UDP_IP = "127.0.0.1"
UDP_PORT = 6000

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP
sock.bind((UDP_IP, UDP_PORT))

data, addr = sock.recvfrom(4 + 3*8)
(i, x, y, psi) = struct.unpack('=Iddd', data)
print("id: {}, x: {}, y: {}, psi: {}".format(i, x, y, psi))
assert i == 1
assert x == 2
assert y == 3
assert psi == -40000

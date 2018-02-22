from subprocess import Popen
import socket
import struct
import time

UDP_IP = "127.0.0.1"
UDP_PORT = 6000

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP
sock.bind((UDP_IP, UDP_PORT))

if __name__ == "__main__":
    # Run tests in C++
    p = Popen(['./test'], cwd='test')

    # receive data from C++ over local UDP
    data, addr = sock.recvfrom(4 + 3*8)
    (i, x, y, psi) = struct.unpack('=Iddd', data)
    print("id: {}, x: {}, y: {}, psi: {}".format(i, x, y, psi))
    assert i == 1
    assert x == 2
    assert y == 3
    assert psi == -40000

    # verify the C++ unit tests pass too
    assert p.wait() == 0

import http.client
import socket as sk
from time import sleep

#=======================================

myhost = 'localhost'
myport = 80

#=======================================

def testOK1():
    print("=====================================")
    with sk.socket(sk.AF_INET, sk.SOCK_STREAM) as s:
        # (host: str, port: int)
        s.connect((myhost, myport))

        s.sendall(b'GET / HTTP/1.1\r\nHost: cs.muic.mahidol.ac.th\r\nConnection: close\r\n\r\n')
        # s.sendall(b'HEAD ')
        # sleep(1)
        # s.sendall(b'/ HTTP/1.1\r')
        # sleep(0.5)
        # s.sendall(b'\n')
        # s.sendall(b'Host: cs.muic.mahidol.ac.th\r\nConnection: close\r\n')
        # sleep(2)
        # s.sendall(b'Abc: yay\r\nWow: hurray\r\n')
        # s.sendall(b'\r\n')

        while data := s.recv(1024):
            print(data)
    print("=====================================")

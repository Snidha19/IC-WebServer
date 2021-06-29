import socket as sk
from time import sleep

with sk.socket(sk.AF_INET, sk.SOCK_STREAM) as s:
    # (host: str, port: int)
    s.connect(('localhost', 80))

    s.sendall(b'GET / HTTP/1.1\r\nHost: www.google.com\r\nConnection: close\r\n\r\n')
    # s.sendall(b'GET ')
    # sleep(1)
    # s.sendall(b'/ HTTP/1.1\r')
    # sleep(0.5)
    # s.sendall(b'\n')
    # s.sendall(b'Host: cs.muic.mahidol.ac.th\r\nConnection: close\r\n')
    # sleep(5)
    # s.sendall(b'\r\n')

    while data := s.recv(1024):
        print(data) 

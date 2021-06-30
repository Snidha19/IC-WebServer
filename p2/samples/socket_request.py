import socket as sk
from time import sleep

with sk.socket(sk.AF_INET, sk.SOCK_STREAM) as s:
    # (host: str, port: int)
    s.connect(('localhost', 12345))

    
    s.sendall(b'HEAD ')
    sleep(1)
    s.sendall(b'/ HTTP/1.1\r')
    sleep(0.5)
    s.sendall(b'\n')
    s.sendall(b'Host: www.google.com\r\nConnection: close\r\n')
    sleep(5)
    s.sendall(b'\r\n')

    while data := s.recv(1024):
        print(data) 

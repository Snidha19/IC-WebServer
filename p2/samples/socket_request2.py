import socket as sk
from time import sleep

with sk.socket(sk.AF_INET, sk.SOCK_STREAM) as s:
    # (host: str, port: int)
    s.connect(('localhost', 12345))

    s.sendall(b'GET /courses/ooc/ HTTP/1.1\r\nHost: cs.muic.mahidol.ac.th\r\nConnection: keep-alive\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\nUser-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/39.0.2171.99 Safari/537.36\r\nAccept-Encoding: gzip, deflate, sdch\r\nAccept-Language: en-US,en;q=0.8\r\n\r\n')
    #s.sendall(b'GET /pcsa/ HTTP/1.1\r\nHost: pcsa.cs.muzoo.io\r\n\r\n')
    # s.sendall(b'HEAD ')
    # sleep(1)
    # s.sendall(b'/ HTTP/1.1\r')
    # sleep(0.5)
    # s.sendall(b'\n')
    # s.sendall(b'Host: www.google.com\r\nConnection: close\r\n')
    # sleep(5)
    # s.sendall(b'\r\n')

    while data := s.recv(1024):
        print(data) 
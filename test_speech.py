import time
import socket
cnt = 0
while 1:
	cnt += 1
	result = "!你好，今天是星期一。" + str(cnt)
	sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
	sock.connect("speech.sock");
	sock.sendall(bytes(result, 'utf-8'))
	time.sleep(2)
	sock.close()

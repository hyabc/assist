import time
import socket
cnt = 0
while 1:
	cnt = cnt + 1
	result = " 你好，今天是星期一。" + str(cnt)
	print(result)
	sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
	sock.connect("speech.sock");
	sock.sendall(bytes(result, 'utf-8'))
	time.sleep(3)
	sock.close()

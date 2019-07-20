import time
import socket
ser = serial.Serial('/dev/ttyACM0', 115200)
while 1:
	result = ser.readline()
	sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
	sock.connect("assist.sock");
	sock.sendall(bytes(result, 'utf-8'))
	sock.close()
ser.close()

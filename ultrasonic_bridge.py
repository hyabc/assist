import time
import serial
import socket
ser = serial.Serial('/dev/ttyACM0', 115200)
for i in range(5):
	result = ser.readline()
while 1:
	result = ser.readline()
	sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
	sock.connect("proxy.sock");
	sock.sendall(result);
	sock.close()
ser.close()

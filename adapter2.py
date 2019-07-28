import time
import serial
import socket
ser = serial.Serial('/dev/ttyACM2', 115200)
while 1:
	result = ser.readline()
	sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
	sock.connect("proxy.sock");
	sock.sendall(result);
	sock.close()
ser.close()

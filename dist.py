import serial
import socket
ser = serial.Serial('/dev/ttyACM0', 115200)
address = ('127.0.0.1', 8888)
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
ser.open()
while 1:
	response = str(ser.readline())
	print(response)
	sock.sendto(response, address)
s.close()

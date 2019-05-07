import serial
import time
import socket
#ser = serial.Serial('/dev/ttyACM0', 115200)
address = ('127.0.0.1', 8888)
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
#ser.open()
while 1:
#response = ser.readline()
	response = bytes("Hello, World!", 'utf-8')
	sock.sendto(response, address)
	time.sleep(1)
#s.close()

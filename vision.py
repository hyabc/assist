import cv2
import socket
import time
sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
sock.connect("assist.sock")
model = cv2.dnn.readNetFromTensorflow('frozen_inference_graph.pb', 'graph.pbtxt')
#Download from https://github.com/opencv/opencv/wiki/TensorFlow-Object-Detection-API
cam = [cv2.VideoCapture(0), cv2.VideoCapture(1)]
cam[0].set(cv2.CAP_PROP_BUFFERSIZE, 3)
cam[1].set(cv2.CAP_PROP_BUFFERSIZE, 3)
cnt = 0
while True:
	cnt = cnt + 1
	#camno = cnt % 2
	camno = 0
	frame = cam[camno].read()[1]
	#frame = cv2.imread("c.jpg")
	height, width, _ = frame.shape
	model.setInput(cv2.dnn.blobFromImage(frame, size=(300, 300), swapRB=True))
	#print("Begin")
	output = model.forward()
	#print("End")
	# print(output[0,0,:,:].shape)
	result = str(camno) + " "
	#cv2.imwrite("cap" + str(cnt) + ".jpg", frame)
	for a in output[0, 0, :, :]:
		if a[2] > .5:
			value = a[2]
			name = a[1]
			x1 = a[3] * width
			y1 = a[4] * height
			dx = a[5] * width
			dy = a[6] * height
			cv2.rectangle(frame, (int(x1), int(y1)), (int(dx), int(dy)), (23, 230, 210), thickness=5)
			result = result + str(a[2]) + ":" + str(name) + " " + str(a[3]) + " " + str(a[4]) + " " + str(a[5]) + " " + str(a[6]) + " "
	cv2.imwrite("detect" + str(cnt) + ".jpg", frame)
	#print(str(cnt) + " " + str(time.localtime(time.time()).tm_sec))
	sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
	sock.connect("assist.sock")
	sock.sendall(bytes(result, 'utf-8'))
	#print(result)

sock.close()

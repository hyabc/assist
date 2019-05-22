import cv2
import time
model = cv2.dnn.readNetFromTensorflow('frozen_inference_graph.pb', 'ssd_mobilenet_v2_coco_2018_03_29.pbtxt')
#Download from https://github.com/opencv/opencv/wiki/TensorFlow-Object-Detection-API
cam = cv2.VideoCapture(0)
while True:
	frame = cam.read()[1]
	#frame = cv2.imread("a.jpg")
	width, height, _ = frame.shape
	model.setInput(cv2.dnn.blobFromImage(frame, size=(150, 150), swapRB=True))
	#print("Begin")
	output = model.forward()
	#print("End")
	# print(output[0,0,:,:].shape)
	for a in output[0, 0, :, :]:
		if a[2] > .5:
			value = a[2]
			name = a[1]
			x1 = a[3] * width
			y1 = a[4] * height
			dx = a[5] * width
			dy = a[6] * height
			cv2.rectangle(frame, (int(x1), int(y1)), (int(dx), int(dy)), (23, 230, 210), thickness=5)
			print(str(a[2]) + ":" + str(name))
	print(time.localtime(time.time()))

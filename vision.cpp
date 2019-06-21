#include "darknet.h"
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <opencv2/dnn.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#define thresh 0.5
#define hier_thresh 0.5
#define nms_thresh 0.4
int main() {
	srand(time(0));
	void* cap = open_video_stream(0, 1, 0, 0, 10);
	//cv::VideoCapture cap(0);
	char **names = get_labels("data/coco.names");
	image **alphabet = load_alphabet();
	//network *net = load_network("cfg/yolov3-tiny.cfg", "yolov3-tiny.weights", 0);
	network *net = load_network("yolov3.cfg", "yolov3.weights", 0);
	set_batch_network(net, 1);
	layer l = net->layers[net->n-1];
	for (int iter = 1;;iter++) {
		printf("BEGIN %d\n", iter);
		time_t start = clock();
		image frame = get_image_from_stream(cap);
		image sized = letterbox_image(frame, net->w, net->h);
		network_predict(net, sized.data);
		int count = 0;
		detection *dets = get_network_boxes(net, frame.w, frame.h, thresh, hier_thresh, 0, 1, &count);
		do_nms_sort(dets, count, l.classes, nms_thresh);
		draw_detections(frame, dets, count, thresh, names, alphabet, l.classes);
		for (int i = 0;i < count;i++) {
			int cur = -1;
			double maxprob = 0.0;
			char* name;
			for (int j = 0;j < l.classes;j++) if (dets[i].prob[j] > thresh && dets[i].prob[j] > maxprob) {cur = j;maxprob = dets[i].prob[j];name = names[j];}
			if (cur != -1) {
				box b = dets[i].bbox;
				int left = (b.x - b.w / 2.0) * frame.w, right = (b.x + b.w / 2.0) * frame.w, top = (b.y - b.h / 2.0) * frame.h, bottom = (b.y + b.h / 2.0) * frame.h;
				printf("(%f, %f) size: (%f, %f) %d~%d, %d~%d, %s, prob=%f\n", b.x, b.y, b.w, b.h, left, right, top, bottom, name, maxprob);
			}
		}
		free_detections(dets, count);
		char buf[10];
		sprintf(buf, "%d", iter);
		save_image(frame, buf);
		free_image(frame);
		free_image(sized);
		printf("%f\n", (double)(clock() - start) / CLOCKS_PER_SEC );
	}
	return 0;
}

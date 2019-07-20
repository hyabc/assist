#include <cstdio>
#include "darknet.h"
#include <ctime>
#include <cstdlib>
#include <opencv2/opencv.hpp>
#define thresh 0.5
#define hier_thresh 0.5
#define nms_thresh 0.4
#define max(x, y) ({x > y ? x : y;})
#define min(x, y) ({x < y ? x : y;})
char buffer[100];
time_t timer;
struct tm* tm_info;
extern "C" {image mat_to_image(cv::Mat);}
int main() {
	srand(time(0));
	cv::VideoCapture cap(1);
	cap.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
	char **names = get_labels("data/coco.names");
	image **alphabet = load_alphabet();
	//network *net = load_network("yolov3-tiny.cfg", "yolov3-tiny.weights", 0);
	network *net = load_network("yolov3.cfg", "yolov3.weights", 0);
	set_batch_network(net, 1);
	layer l = net->layers[net->n-1];
	for (int iter = 1;;iter++) {
		printf("BEGIN %d\n", iter);
		time_t start = clock();
		cv::Mat f;
		//cap >> f;
		f = cv::imread("w.jpg");
		image frame = mat_to_image(f);
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
				left = max(0, left);
				right = min(frame.w - 1, right);
				top = max(0, top);
				bottom = min(frame.h - 1, bottom);
				cv::Rect region;
				region.x = left;
				region.y = top;
				region.width = right - left;
				region.height = bottom - top;
				cv::Mat graph = f(region);
				printf("(%f, %f) size: (%f, %f) %d~%d, %d~%d, %s, prob=%f\n", b.x, b.y, b.w, b.h, left, right, top, bottom, name, maxprob);
				char buf[100];
				sprintf(buf, "%d_detect%d.jpg", iter, i);
				imwrite(buf, graph);
			}
		}
		free_detections(dets, count);
		char buf[10];
		sprintf(buf, "%d", iter);
		save_image(frame, buf);
		free_image(frame);
		free_image(sized);
		printf("%f\n", (double)(clock() - start) / CLOCKS_PER_SEC);

		time(&timer);
		tm_info = localtime(&timer);
		strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
		puts(buffer);

	}
	return 0;
}

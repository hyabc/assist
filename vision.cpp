#include "darknet.h"
#include <cstdio>
#include <ctime>
#include <cstdlib>
#include <opencv2/opencv.hpp>
extern "C" {
image mat_to_image(cv::Mat);
}
extern "C" {
#include "assist.h"
}

#define thresh 0.5
#define hier_thresh 0.5
#define nms_thresh 0.4
#define max(x, y) ({x > y ? x : y;})
#define min(x, y) ({x < y ? x : y;})

char buf[MAXBUF];

namespace traffic_light {
	double area;
	int cur_option;

	void judge(cv::Mat src, int option) {
		cv::Mat mid, mask1, mask2, obj;
		cv::cvtColor(src, mid, cv::COLOR_BGR2HSV);
		if (option == 0) {
			cv::inRange(mid, cv::Scalar(160, 64, 100), cv::Scalar(179, 255, 255), mask1);
			cv::inRange(mid, cv::Scalar(0, 64, 100), cv::Scalar(20, 255, 255), mask2);
			cv::bitwise_or(mask1, mask2, obj);
		} else
			cv::inRange(mid, cv::Scalar(60, 64, 100), cv::Scalar(90, 255, 255), obj);

		std::vector<std::vector<cv::Point> > contours;
		cv::findContours(obj, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

		if (contours.size() == 0) return;

		int cur = 0;
		for (int i = 0;i < contours.size();i++) if (cv::contourArea(contours[i]) > cv::contourArea(contours[cur])) cur = i;

		if (cv::contourArea(contours[cur]) < 40) return;

		cv::Rect rcur = cv::boundingRect(cv::Mat(contours[cur]));
		double x = (double)(rcur.height) / rcur.width;
		if (x < 1.5) return;
		if (cur_option == -1 || cv::contourArea(contours[cur]) > area) {
			area = cv::contourArea(contours[cur]);
			cur_option = option;
		}
	}
}
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

	for (int iter = 1;/*iter <= 1*/;iter++) {
		traffic_light::cur_option = -1;

//		printf("BEGIN %d\n", iter);
//		time_t start = clock();
		cv::Mat f;
		cap >> f;
//		f = cv::imread("a.jpg");
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
			for (int j = 0;j < l.classes;j++)
				if (dets[i].prob[j] > thresh && dets[i].prob[j] > maxprob) {
					cur = j;
					maxprob = dets[i].prob[j];
					name = names[j];
				}

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
				cv::Mat subgraph = f(region);
				printf("(%f, %f) size: (%f, %f) %d~%d, %d~%d, %s, prob=%f\n", b.x, b.y, b.w, b.h, left, right, top, bottom, name, maxprob);

				sprintf(buf, "%d_detect%d.jpg", iter, i);
//				imwrite(buf, subgraph);

				if (strcmp(name, "traffic light") == 0) {
					traffic_light::judge(subgraph, 0);
					traffic_light::judge(subgraph, 1);
				}
					
			}
		}
		sprintf(buf, "%d", iter);
//		save_image(frame, buf);

		free_detections(dets, count);
		free_image(frame);
		free_image(sized);
//		printf("%f\n", (double)(clock() - start) / CLOCKS_PER_SEC);

		if (traffic_light::cur_option != -1) 
			printf("Color: %s\n", traffic_light::cur_option == 0 ? "Red" : "Green");
		
		sprintf(buf, "V%d", traffic_light::cur_option);
		submit("proxy.sock", buf);
	}
	return 0;
}

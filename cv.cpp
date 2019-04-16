#include <opencv2/opencv.hpp>
#include <cstdio>
#include <vector>
using namespace cv;
int main() {
	VideoCapture cap(0);
	Mat frame;
	cap >> frame;
	imwrite("cv.jpg", frame);
	return 0;
}

#include <opencv2/opencv.hpp>
#include <cstdio>
#include <vector>
using namespace cv;
int main() {
	VideoCapture cap(0);
	Mat frame;
	cap >> frame;
	cv::CascadeClassifier carClassifier;
	carClassifier.load("carClassifier.xml");
	std::vector<cv::Rect> cars;
	imwrite("cv.jpg", frame);
	carClassifier.detectMultiScale(frame, cars, 1.1, 5, 0, cv::Size(20, 20) );
	Mat result = frame.clone();
	for (std::vector<cv::Rect>::iterator iter = cars.begin();iter != cars.end();iter++) {
		printf("%d %d %d %d\n", iter->x, iter->x + iter->width, iter->y, iter->y + iter->height);
				cv::rectangle(
									result,
												cv::Point(iter->x, iter->y),
															cv::Point(iter->x + iter->width, iter->y + iter->height),
																		CV_RGB(255, 0, 0),
																					2);
	}
	imwrite("result.jpg", result);
	return 0;
}

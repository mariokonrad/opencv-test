#include <iostream>
#include <signal.h>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

volatile bool do_quit = false;

extern "C" void sigint_handler(int) { do_quit = true; }

int main()
{
	signal(SIGINT, sigint_handler);

	VideoCapture webcam(0);

	if (!webcam.isOpened()) {
		std::cout << "error: CANNOT OPEN CAM\n";
		return -1;
	}

	std::cout << "frame size: width=" << webcam.get(CV_CAP_PROP_FRAME_WIDTH)
			  << " height=" << webcam.get(CV_CAP_PROP_FRAME_HEIGHT) << "\n";

	namedWindow("edges", CV_WINDOW_AUTOSIZE);

	int filter = 0;

	while (true) {
		try {
			Mat frame;
			if (!webcam.read(frame)) {
				std::cout << "error: cannot read frame from device\n";
				break;
			}
			if (do_quit)
				break;
			if (!frame.empty()) {
				switch (filter) {
					case 0:
						break;

					case 1: {
						GaussianBlur(frame, frame, Size(7, 7), 1.5, 1.5);
						Canny(frame, frame, 0, 30, 3);
					} break;

					case 2: {
						GaussianBlur(frame, frame, Size(7, 7), 1.5, 1.5);
						Mat gray;
						cvtColor(frame, gray, CV_BGR2GRAY);
						std::vector<Vec3f> circles;
						HoughCircles(
							gray, circles, CV_HOUGH_GRADIENT, 2, gray.rows / 4, 200, 100, 0, 0);
						for (size_t i = 0; i < circles.size(); ++i) {
							Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
							int radius = cvRound(circles[i][2]);
							// draw the circle center
							circle(frame, center, 3, Scalar(0, 255, 0), -1, 8, 0);
							// draw the circle outline
							circle(frame, center, radius, Scalar(0, 0, 255), 3, 8, 0);
						}
					} break;

					case 3: {
						GaussianBlur(frame, frame, Size(7, 7), 1.5, 1.5);
						Canny(frame, frame, 10, 200, 3);
						std::vector<Vec4i> lines;
						HoughLinesP(frame, lines, 1, CV_PI / 180, 80, 30, 10);
						for (size_t i = 0; i < lines.size(); i++) {
							line(frame, Point(lines[i][0], lines[i][1]),
								Point(lines[i][2], lines[i][3]), Scalar(0, 0, 255), 3, 8);
						}
					} break;
				}

				// display frame
				imshow("edges", frame);
			}

			int key = waitKey(20);
			if (key >= 0) {
				key &= 0xff;
				if (key == 27)
					break;
				if (key == '0')
					filter = 0;
				if (key == '1')
					filter = 1;
				if (key == '2')
					filter = 2;
				if (key == '3')
					filter = 3;
			}
		} catch (Exception & e) {
			std::cout << "error: " << e.what() << "\n";
			return -1;
		}
	}
	return 0;
}

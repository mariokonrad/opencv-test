#include <iostream>
#include <signal.h>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

volatile bool do_quit = false;

extern "C" void sigint_handler(int) { do_quit = true; }

template <class T> inline T sqr(T a) noexcept { return a * a; }

int main()
{
	signal(SIGINT, sigint_handler);

	VideoCapture webcam(0);

	if (!webcam.isOpened()) {
		std::cout << "error: CANNOT OPEN CAM\n";
		return -1;
	}

	const int width = webcam.get(CV_CAP_PROP_FRAME_WIDTH);
	const int height = webcam.get(CV_CAP_PROP_FRAME_HEIGHT);

	std::cout << "frame size: width=" << width << " height=" << height << "\n";

	namedWindow("cam", CV_WINDOW_AUTOSIZE);
	//namedWindow("motion", CV_WINDOW_AUTOSIZE);

	Mat frame0;
	if (!webcam.read(frame0)) {
		std::cout << "error: cannot read frame from device\n";
		return -1;
	}

	while (true) {
		try {
			// read data
			Mat frame1;
			if (!webcam.read(frame1)) {
				std::cout << "error: cannot read frame from device\n";
				break;
			}

			// exit by ctrl-c?
			if (do_quit)
				break;

			// key processing
			int key = waitKey(20);
			if (key >= 0) {
				key &= 0xff;
				if ((key == 27) || (key == 'q') || (key == 'Q'))
					break;
			}

			if (frame0.empty() || frame1.empty()) {
				std::cout << "error: invalid frames\n";
				break;
			}

			// filter
			Mat frame0_gray;
			Mat frame1_gray;
			cvtColor(frame0, frame0_gray, COLOR_BGR2GRAY);
			cvtColor(frame1, frame1_gray, COLOR_BGR2GRAY);

			// compute difference
			Mat diff;
			absdiff(frame0_gray, frame1_gray, diff);
			Mat threshold_image;
			threshold(diff, threshold_image, 20, 255, THRESH_BINARY);

			// get rid of noise
			blur(threshold_image, threshold_image, Size(10, 10));
			threshold(threshold_image, threshold_image, 20, 255, THRESH_BINARY);

			// compute motion
			Mat tmp;
			threshold_image.copyTo(tmp);
			vector<vector<Point>> contours;
			vector<Vec4i> hierarchy;
			findContours(tmp, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

			Mat frame_copy;
			frame1.copyTo(frame_copy);
			for (auto const & c : contours) {
				auto rect = boundingRect(c);
				rectangle(frame_copy, rect, Scalar(0, 0, 255));
			}

			// output
			imshow("cam", frame_copy);
			//imshow("motion", tmp);

			// save current image
			frame0 = frame1;

		} catch (Exception & e) {
			std::cout << "error: " << e.what() << "\n";
			return -1;
		}
	}
	return 0;
}

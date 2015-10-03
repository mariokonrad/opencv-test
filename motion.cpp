#include <iostream>
#include <signal.h>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

volatile bool do_quit = false;

extern "C" void sigint_handler(int) { do_quit = true; }

template <class T> inline T sqr(T a) noexcept { return a * a; }

static int threshold_value = 20;
static int noise_blur = 10;
static int min_area = 30 * 30;
static bool motion_window_visible = false;

static const std::string win_title_cam = "cam";
static const std::string win_title_options = "options";
static const std::string win_title_motion = "motion";

static void on_trackbar(int value, void * ptr)
{
	if (ptr == &noise_blur) {
		if (value < 1)
			setTrackbarPos("noise blur", win_title_options, 1);
	}
}

static void create_options_window()
{
	namedWindow(win_title_options, WINDOW_AUTOSIZE);
	createTrackbar("threshold", win_title_options, &threshold_value, 30, on_trackbar);
	createTrackbar("noise blur", win_title_options, &noise_blur, 20, on_trackbar, &noise_blur);
	createTrackbar("min area", win_title_options, &min_area, 1600, on_trackbar, &min_area);
}

static void toggle_options_window()
{
	static bool visible = false;

	if (visible) {
		destroyWindow(win_title_options);
	} else {
		create_options_window();
	}
	visible = !visible;
}

static void toggle_motion_window()
{
	if (motion_window_visible) {
		destroyWindow(win_title_motion);
	} else {
		namedWindow(win_title_motion, CV_WINDOW_AUTOSIZE);
	}
	motion_window_visible = !motion_window_visible;
}

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

	namedWindow(win_title_cam, CV_WINDOW_AUTOSIZE);

	Mat frame0;
	if (!webcam.read(frame0)) {
		std::cout << "error: cannot read frame from device\n";
		return -1;
	}
	Mat frame0_gray;
	cvtColor(frame0, frame0_gray, COLOR_BGR2GRAY);

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
				if (key == 'o')
					toggle_options_window();
				if (key == 'm')
					toggle_motion_window();
			}

			if (frame0.empty() || frame1.empty()) {
				std::cout << "error: invalid frames\n";
				break;
			}

			// filter
			Mat frame1_gray;
			cvtColor(frame1, frame1_gray, COLOR_BGR2GRAY);

			// compute difference
			Mat diff;
			absdiff(frame0_gray, frame1_gray, diff);
			Mat threshold_image;
			threshold(diff, threshold_image, threshold_value, 255, THRESH_BINARY);

			// get rid of noise
			blur(threshold_image, threshold_image, Size(noise_blur, noise_blur));
			threshold(threshold_image, threshold_image, threshold_value, 255, THRESH_BINARY);

			// compute contours
			Mat tmp;
			threshold_image.copyTo(tmp);
			vector<vector<Point>> contours;
			vector<Vec4i> hierarchy;
			findContours(tmp, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

			// draw rectangles arround moving parts
			Mat frame_copy;
			frame1.copyTo(frame_copy);
			for (auto const & c : contours) {
				const Moments moment = moments(c);
				double area = moment.m00;
				if (area >= static_cast<double>(min_area)) {
					auto rect = boundingRect(c);
					rectangle(frame_copy, rect, Scalar(0, 0, 255));
				}
			}

			// output
			imshow(win_title_cam, frame_copy);
			if (motion_window_visible)
				imshow(win_title_motion, tmp);

			// save current image
			frame0 = frame1;
			frame0_gray = frame1_gray;

		} catch (Exception & e) {
			std::cout << "error: " << e.what() << "\n";
			return -1;
		}
	}
	return 0;
}

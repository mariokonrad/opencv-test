#include <stdio.h>
#include <cv.h>
#include <highgui.h>

int main()
{
	// Allocates and initializes cvCapture structure
	// for reading a video stream from the camera.
	// Index of camera is -1 since only one camera
	// connected to the computer or it does not
	// matter what camera to use.
	CvCapture * input_camera = cvCaptureFromCAM(-1);

	// Grabs and returns a frame from camera
	IplImage * frame = cvQueryFrame(input_camera);

	// Creates window for displaying the frames
	// Flag is reset (0) --> change window size
	// manually
	cvNamedWindow("Capturing Image ...", 0);

	// Change to the appropriate size. In GTK, the
	// inappropriate size will return a segmentation
	// fault. I don't know why ...
	// Gets the appropriate size using cvGetCaptureProperty
	// with CV_CAP_PROP_FRAME_HEIGHT and CV_CAP_PROP_FRAME_WIDTH
	// as property_id
	cvResizeWindow("Capturing Image ...",
		(int)cvGetCaptureProperty(input_camera, CV_CAP_PROP_FRAME_HEIGHT),
		(int)cvGetCaptureProperty(input_camera, CV_CAP_PROP_FRAME_WIDTH));

	while (frame) {
		// Shows a frame
		cvShowImage("Capturing Image ...", frame);

		// Checks if ESC is pressed and gives a delay
		// so that the frame can be displayed properly
		if (cvWaitKey(10) == 27)
			break;

		// Grabs and returns the next frame
		frame = cvQueryFrame(input_camera);
	}

	// Release cvCapture structure
	cvReleaseCapture(&input_camera);

	// Destroy the window
	cvDestroyWindow("Capturing Image ...");

	return 0;
}

#include <OpenNI.h>
#include <opencv2/opencv.hpp>
#include <vector>

int main() {
    try {
    	openni::OpenNI::initialize();
   	openni::Device device;
		int ret = device.open(openni::ANY_DEVICE);
		if (ret!=openni::STATUS_OK) {
			return 0;
  		}

		openni::VideoStream depthStream;
		depthStream.create(device, openni::SENSOR_DEPTH);
		depthStream.start();

		cvNamedWindow("Image", CV_WINDOW_AUTOSIZE);
		cvResizeWindow("Image", 800,600);
		std::vector<openni::VideoStream*> streams;
		streams.push_back(&depthStream);

		cv::Mat depthImage;

		while(1) {
			int changedIndex;
			openni::OpenNI::waitForAnyStream(&streams[0], streams.size(),&changedIndex);
			if (changedIndex == 0) {
				openni::VideoFrameRef colorFrame;
				depthStream.readFrame(&colorFrame);
				if (colorFrame.isValid()) {
					depthImage = cv::Mat(depthStream.getVideoMode().getResolutionY(),
						depthStream.getVideoMode().getResolutionX(),
						CV_16U, (char*)colorFrame.getData());
					depthImage.convertTo(depthImage, CV_8U);
					cv::imshow("Image", depthImage);
				}
			}

			int key = cv::waitKey(10);
			if (key=='q') {
				break;
			}
		}	
	}

	catch(std::exception&) {}
	return 0;
}

#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <windows.h>
#include "Instance.h"

using namespace std;

int main()
{
	cv::namedWindow("Opa, meu mel.", cv::WINDOW_AUTOSIZE);
	Instance instance("LV60A.dat", 300, 300, 300);
	//Instance instance("Bentheimer.raw", 1000, 1000, 1000);

	instance.reverse();
	instance.distanceMap();
	instance.gaussianFilter(3);
	
	Instance peaksIdentified(instance.peakIdentify(5));
	peaksIdentified.normalize(255.);
	instance.normalize(255.);
	
	int d = 0;
	for (;;) {
		cv::Mat resized;
		cv::resize(instance.rock.at(d), resized, cv::Size(), 2., 2.);
		cv::imshow("g_filtered", resized);

		cv::Mat resized2;
		cv::resize(peaksIdentified.rock.at(d), resized2, cv::Size(), 2., 2.);
		cv::imshow("peaks_identified", resized2);

			d++;
		if (d == instance.DEPTH)
			d = 0;
		char c = cv::waitKey(100);
		if (c == 27) break;
	}
	cv::waitKey(0);

	return 0;
}
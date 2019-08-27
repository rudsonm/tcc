#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <windows.h>
#include "Instance.h"
#include "Watershed.h"
#include <conio.h>

using namespace std;

int main()
{
	cv::namedWindow("labeled", cv::WINDOW_AUTOSIZE);
	Instance instance("LV60A_amostra_50.dat", 50, 50, 50);
	//Instance instance("LV60A_amostra.dat", 100, 100, 100);
	//Instance instance("LV60A.dat", 300, 300, 300);
	//Instance instance("Bentheimer.raw", 1000, 1000, 1000);

	instance.reverse();
	instance.distanceMap();
	instance.gaussianFilter(3);
	
	Instance peaksIdentified(instance.peakIdentify(10));

	instance.peaks = peaksIdentified.labeling(true);	
	instance.peaks = instance.removePeaksOnSaddles();

	instance.peaks = instance.mergePeaks();

	peaksIdentified.updatePeaks(instance.peaks);

	Instance watersheded(Watershed::segmentate(instance));

	instance.normalize(255.);
	int d = 0;	
	cv::createTrackbar("Camada", "labeled", &d, instance.DEPTH - 1);
	for (;;) {
		cv::Mat resized;
		cv::resize(instance.rock.at(d), resized, cv::Size(600, 600), 1., 1.);
		cv::imshow("g_filtered", resized);

		cv::Mat resized2;
		cv::resize(peaksIdentified.rock.at(d), resized2, cv::Size(600, 600), 1., 1.);
		cv::imshow("labeled", resized2);

		cv::Mat resized3;
		cv::resize(watersheded.rock.at(d), resized3, cv::Size(600, 600), 1., 1.);
		cv::imshow("segmented", resized3);

		char c = cv::waitKey(1000);
		if (c == 119) {
			d++;
		} else if (c == 115) {
			d--;
		} else if (c == 27) {
			break;
		}

		if (d == instance.DEPTH) {
			d = 0;
		} else if (d < 0) {
			d = instance.DEPTH - 1;
		}
		cv::setTrackbarPos("Camada", "labeled", d);
	}
	cv::waitKey(0);

	return 0;
}
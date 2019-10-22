#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <windows.h>
#include "Instance.h"
#include "Watershed.h"
#include "GraphExtractor.h"
#include <conio.h>

using namespace std;

int main()
{	
	Instance instance("LV60A_amostra_50.dat", 50, 50, 50);
	//Instance instance("LV60A_amostra.dat", 100, 100, 100);
	//Instance instance("C1_amostra_100.dat", 100, 100, 100);
	//Instance instance("LV60A.dat", 300, 300, 300);
	//Instance instance("Berea.dat", 400, 400, 400);
	//Instance instance("C1.dat", 400, 400, 400);
	//Instance instance("Bentheimer.raw", 1000, 1000, 1000);

	instance.reverse();
	instance.distanceMap();	

	Instance gaussianFiltered(instance.rock);
	gaussianFiltered.gaussianFilter(3);

	Instance peaksIdentified(gaussianFiltered.peakIdentify(10));

	gaussianFiltered.peaks = peaksIdentified.labeling(true);
	gaussianFiltered.removePeaksOnSaddles();
	instance.peaks = gaussianFiltered.mergePeaks();

	Instance watersheded(Watershed::segmentate(instance));
	watersheded.peaks = instance.peaks;

	Graph graph = GraphExtractor::extract(watersheded);	
	double maximumFlow = graph.getPushRelabelMaximumFlow(NORTH, SOUTH);
	printf("Maximum flow: %f\n", maximumFlow);

	watersheded.rock = Watershed::paint(watersheded.rock);
	gaussianFiltered.normalize(255.);

	int d = 0;
	cv::namedWindow("watershed", cv::WINDOW_AUTOSIZE);
	cv::createTrackbar("Slice", "watershed", &d, instance.DEPTH - 1);
	for (;;) {
		cv::Mat resized;
		cv::resize(peaksIdentified.rock.at(d), resized, cv::Size(400, 400), 1., 1.);
		cv::imshow("max_filter", resized);

		cv::Mat resized2;
		cv::resize(watersheded.rock.at(d), resized2, cv::Size(400, 400), 1., 1.);
		cv::imshow("watershed", resized2);

		cv::Mat resized3;
		cv::resize(gaussianFiltered.rock.at(d), resized3, cv::Size(400, 400), 1., 1.);
		cv::imshow("gaussian_filter", resized3);

		char c = cv::waitKey(0);
		if (c == 119)
			d++;
		else if (c == 115)
			d--;
		else if (c == 27)
			break;

		if (d == instance.DEPTH)
			d = 0;
		else if (d < 0)
			d = instance.DEPTH - 1;
		cv::setTrackbarPos("Camada", "labeled", d);		
	}
	cv::waitKey(0);
	return 0;
}
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
	float viscosity = 0.00103;
	float resolution = 5.345;

	//Instance instance("LV60A_amostra_50.dat", 50, 50, 50);
	//Instance instance("LV60A_amostra.dat", 100, 100, 100);
	//Instance instance("C1_amostra_100.dat", 100, 100, 100);
	//Instance instance("LV60A.dat", 300, 300, 300);
	Instance instance("Berea.dat", 400, 400, 400);
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

	//Graph graph;
	//graph.add(1);
	//graph.add(2);
	//graph.add(3);
	//graph.add(4);
	//graph.add(5);
	//graph.add(6);

	//graph.addEdge(1, 2, 15);
	//graph.addEdge(1, 3, 4);
	//graph.addEdge(2, 4, 12);
	//graph.addEdge(2, 5, 5);
	//graph.addEdge(3, 4, 3);
	//graph.addEdge(3, 5, 10);
	//graph.addEdge(4, 6, 7);
	//graph.addEdge(5, 6, 10);

	//graph.getVertex(1)->directions = std::vector<int>(1, NORTH);
	//graph.getVertex(1)->surface = 13;

	//graph.getVertex(6)->directions = std::vector<int>(6, SOUTH);
	//graph.getVertex(6)->surface = 17;

	//double maximumFlow = graph.getPushRelabelMaximumFlow(NORTH, SOUTH);
	//printf("Maximum flow: %f\n", maximumFlow);

	//for (Vertex v : graph.vertices) {
	//	printf("vertex: %i\n", v.voxel.label);
	//	for (auto const& edge : graph.adjacentList[v.voxel.label]) {
	//		printf("%i -> %i = %i\n", edge.second.source, edge.second.target, edge.second.flow);
	//	}
	//	printf("\n");
	//}	

	Graph graph = GraphExtractor::extract(watersheded, instance, viscosity, resolution);
	float maximumFlow = graph.getPushRelabelMaximumFlow(NORTH, SOUTH);	
	printf("Maximum flow: %f\n", maximumFlow);
	printf("Surface: %f\n", graph.getSurfaceArea(SOUTH));
	float permeability = (maximumFlow * viscosity * resolution) / graph.getSurfaceArea(SOUTH);
	printf("PERMEABILIDADEEEEEEEEE = %f\n", permeability);

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
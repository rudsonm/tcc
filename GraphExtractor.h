#pragma once
#include "Graph.h"
#include "Instance.h"
#include "Watershed.h"
#include "PDIUtils.h"

class GraphExtractor
{
public:
	static Graph extract(Instance &instance, const Instance &distanceMap, float viscosity, float resolution);
	static std::vector<int> isSurfaceVertex(const Voxel &v, int DEPTH, int HEIGHT, int WIDTH) {
		std::vector<int> directions;
		if (v.z == 0)
			directions.push_back(NORTH);
		if (v.z == DEPTH - 1)
			directions.push_back(SOUTH);
		if (v.y == 0)
			directions.push_back(WEST);
		if (v.y == WIDTH - 1)
			directions.push_back(EAST);
		if (v.x == 0)
			directions.push_back(BACK);
		if (v.x == HEIGHT - 1)
			directions.push_back(FRONT);
		return directions;
	}
	static void exportToGephi(Graph graph, std::string fileName);
	static void exportToPNM(Graph graph, std::string fileName);
};


#include "GraphExtractor.h"
#include <math.h>

Graph GraphExtractor::extract(Instance &instance, const Instance &distanceMap, float viscosity, float resolution)
{
	printf("Extracting graph\n");
	Graph graph;
	graph.DEPTH = instance.DEPTH;
	graph.HEIGHT = instance.HEIGHT;
	graph.WIDTH = instance.WIDTH;
	for (auto const& peak : instance.peaks) {
		int x = peak.second.at(0).x;
		int y = peak.second.at(0).y;
		int z = peak.second.at(0).z;

		int radius = distanceMap.rock.at(z).at<uchar>(x, y);

		Voxel voxel = PDIUtils::getCentroid(peak.second);
		voxel.label = peak.first;
		graph.add( Vertex(voxel, radius) );
	}
	printf("Added %i vertices\n", graph.vertices.size());	
	for (int z = 0; z < instance.DEPTH; z++) {		
		for (int x = 0; x < instance.HEIGHT; x++) {
			for (int y = 0; y < instance.WIDTH; y++) {
				ushort label = instance.rock.at(z).at<ushort>(x, y);
				if (label == Watershed::WATERSHED_BARRIER) {
					int distanceMapValue = distanceMap.rock.at(z).at<uchar>(x, y);

					std::set<ushort> neighbors = instance.getNeighborsLabels(z, x, y);
					std::vector<ushort> neighborsVector(neighbors.begin(), neighbors.end());
					for (int i = 0; i < neighborsVector.size(); i++) {
						ushort source = neighborsVector[i];
						for (int j = i + 1; j < neighborsVector.size(); j++) {
							ushort target = neighborsVector[j];
							if (source == Watershed::WATERSHED_BARRIER || target == Watershed::WATERSHED_BARRIER)
								continue;																				

							graph.adjacentList[source][target].weight++;
							graph.adjacentList[source][target].source = source;
							graph.adjacentList[source][target].target = target;

							graph.adjacentList[target][source].weight++;
							graph.adjacentList[target][source].source = target;
							graph.adjacentList[target][source].target = source;

							if (graph.adjacentList[source][target].weight == 1) {
								graph.connections += 2;
							}

							if (distanceMapValue > graph.adjacentList[source][target].radius) {
								graph.adjacentList[source][target].radius = distanceMapValue;
								graph.adjacentList[source][target].centroid = Voxel(z, x, y);

								graph.adjacentList[target][source].radius = distanceMapValue;
								graph.adjacentList[target][target].centroid = Voxel(z, x, y);
							}
						}
					}
				} else if (label > 0) {
					graph.sumVolume(label);
					graph.setVertexDirections(label, isSurfaceVertex(Voxel(z, x, y), instance.DEPTH, instance.HEIGHT, instance.WIDTH));
				}
			}
		}		
	}

	for (Vertex v : graph.vertices) {
		std::vector<int> toErase;
		for (const auto& edge : graph.adjacentList[v.voxel.label]) {
			int source = graph.adjacentList[v.voxel.label][edge.first].source;
			int target = graph.adjacentList[v.voxel.label][edge.first].target;
			if (source == target) {
				toErase.push_back(edge.first);
			}
		}
		for (int target : toErase) {
			graph.adjacentList[v.voxel.label].erase(target);
		}
	}

	for (Vertex &v : graph.vertices) {
		v.surface *= pow(resolution, 2);
		for (const auto &edge : graph.adjacentList[v.voxel.label]) {
			float diameter = edge.second.radius * 2. * resolution;

			Vertex vtarget = *graph.getVertex(edge.second.target);

			float distance = PDIUtils::getEuclideanDistance(v.voxel, edge.second.centroid);
			distance += PDIUtils::getEuclideanDistance(edge.second.centroid, vtarget.voxel);
			distance -= (v.radius + vtarget.radius);
			distance *= resolution;

			graph.adjacentList[v.voxel.label][edge.second.target].weight = (M_PI / (128. * viscosity)) * ( pow(diameter, 4) / distance );
		}
	}
	//printf("%i Edges found\n", graph.adjacentList.size());
	return graph;
}

void GraphExtractor::exportToGephi(Graph graph, std::string fileName) {
	std::ofstream file(fileName + ".net");
	file << "*vertices " << graph.vertices.size() << "\n";
	for (Vertex vertice : graph.vertices)
		file << graph.getVertexIndex(vertice.voxel.label) + 1 
			<< " " << vertice.voxel.label
			<< " " << vertice.voxel.z
			<< " " << vertice.voxel.x
			<< " " << vertice.voxel.y
			<< " " << vertice.volume << " " 
			<< (vertice.surface ? "1" : "0") << "\n";
	file << "*arcs\n";
	for (const auto& edge : graph.adjacentList) {
		int source = graph.getVertexIndex(edge.first) + 1;
		for (const auto& edgeTarget : edge.second) {
			int target = graph.getVertexIndex(edgeTarget.first) + 1;
			file << source << " " << target << " " << edgeTarget.second.weight << "\n";
		}
	}
	file.close();
}

void GraphExtractor::exportToPNM(Graph graph, std::string fileName) {
	std::ofstream file(fileName + ".pnm");
	file << graph.HEIGHT << " " << graph.WIDTH << " " << graph.DEPTH << "\n";
	for (Vertex vertice : graph.vertices)
		file << graph.getVertexIndex(vertice.voxel.label) + 1
		<< " " << vertice.voxel.z
		<< " " << vertice.voxel.x
		<< " " << vertice.voxel.y
		<< " " << vertice.volume << " "
		<< (vertice.surface ? "1" : "0") << "\n";
	for (const auto& edge : graph.adjacentList) {
		int source = graph.getVertexIndex(edge.first) + 1;
		for (const auto& edgeTarget : edge.second) {
			int target = graph.getVertexIndex(edgeTarget.first) + 1;
			file << source << " " << target << " " << edgeTarget.second.weight << "\n";
		}
	}
	file.close();
}
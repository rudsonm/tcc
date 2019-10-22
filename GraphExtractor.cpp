#include "GraphExtractor.h"

Graph GraphExtractor::extract(Instance &instance)
{
	printf("Extracting graph\n");
	Graph graph;
	graph.DEPTH = instance.DEPTH;
	graph.HEIGHT = instance.HEIGHT;
	graph.WIDTH = instance.WIDTH;
	for (auto const& peak : instance.peaks) {
		Voxel voxel = PDIUtils::getCentroid(peak.second);
		voxel.label = peak.first;
		graph.add(Vertex(voxel));
	}
	printf("Added %i vertices\n", graph.vertices.size());	
	for (int z = 0; z < instance.DEPTH; z++) {		
		for (int x = 0; x < instance.HEIGHT; x++) {
			for (int y = 0; y < instance.WIDTH; y++) {
				ushort label = instance.rock.at(z).at<ushort>(x, y);
				if (label == Watershed::WATERSHED_BARRIER) {
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
							if (graph.adjacentList[source][target].weight == 1)
								graph.connections += 2;
						}
					}
				} else if (label > 0) {
					graph.sumVolume(label);
					graph.setVertexDirections(label, isSurfaceVertex(Voxel(z, x, y), instance.DEPTH, instance.HEIGHT, instance.WIDTH));
				}
			}
		}
	}
	printf("%i Edges found\n", graph.adjacentList.size());
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
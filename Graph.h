#pragma once
#include "Voxel.h"
#include <map>
#include <queue>

#define NORTH 1
#define EAST 2
#define SOUTH 3
#define WEST 4
#define FRONT 5
#define BACK 6

struct Edge {
	int source, target, length;
	float weight = 0., flow = 0.;
	float radius = 0.;
	Voxel centroid;
};

struct Vertex {
	Voxel voxel;
	float surface = 0;
	int volume = 0;
	int radius = 0;
	std::vector<int> directions;
	Vertex(Voxel voxel) : voxel(voxel) {}
	Vertex(Voxel voxel, int radius) : voxel(voxel), radius(radius) {}
	bool isInDirection(int direction) {
		for (int dir : directions)
			if (dir == direction)
				return true;
		return false;
	}
};

using AdjacentList = std::map<int, std::map<int, Edge>>;
using Vertices = std::vector<Vertex>;

class Graph
{
public:
	Vertices vertices;
	AdjacentList adjacentList;
	int connections = 0;
	int DEPTH;
	int WIDTH;
	int HEIGHT;

	void add(Vertex vertex) {
		vertices.push_back(vertex);
		adjacentList[vertex.voxel.label] = std::map<int, Edge>();
	}

	void add(int label) {
		Voxel v(0, 0, 0);
		v.label = label;
		Vertex vertex(v);
		this->add(vertex);
	}

	void addEdge(int u, int v, float weight) {
		adjacentList[u][v].weight = weight;
		adjacentList[u][v].source = u;
		adjacentList[u][v].target = v;

		adjacentList[v][u].weight = weight;
		adjacentList[v][u].source = v;
		adjacentList[v][u].target = u;
	}

	Vertex *getVertex(int label) {
		for (Vertex& vertex : vertices)
			if (vertex.voxel.label == label)
				return &vertex;
		throw "Vertex not found.";
	}

	int getVertexIndex(int label) {
		for (int i = 0; i < vertices.size(); i++)
			if (vertices[i].voxel.label == label)
				return i;
		return -1;
	}

	void sumWeight(int source, int target) {
		adjacentList[source][target].weight++;
	}

	void sumVolume(int label) {
		getVertex(label)->volume++;
	}

	void setVertexDirections(int label, std::vector<int> directions) {
		Vertex* vertex = getVertex(label);
		if (directions.size() > 0) {
			vertex->surface++;
			for (int d1 : directions) {
				bool exists = false;
				for (int d2 : vertex->directions) {
					if (d1 == d2) {
						exists = true;
					}
				}
				if (!exists) {
					vertex->directions.push_back(d1);
				}
			}
		}		
	}

	int getAvailableLabel() {
		int label = 0;
		bool inUse = false;

	checkNextLabel:
		label++;
		for (Vertex vertex : vertices)
			if (vertex.voxel.label == label)
				goto checkNextLabel;
		return label;
	}

	Vertex setArtificialVertex(int direction) {
		Voxel voxel(-1, -1, -1);
		voxel.label = this->getAvailableLabel();
		Vertex artificialVertex(voxel);
		this->add(artificialVertex);
		float totalWeight = 0.;
		for (Vertex vertex : vertices) {
			if (vertex.isInDirection(direction)) {
				adjacentList[artificialVertex.voxel.label][vertex.voxel.label] = Edge();
				adjacentList[artificialVertex.voxel.label][vertex.voxel.label].source = artificialVertex.voxel.label;
				adjacentList[artificialVertex.voxel.label][vertex.voxel.label].target = vertex.voxel.label;
				adjacentList[artificialVertex.voxel.label][vertex.voxel.label].weight = vertex.surface;

				adjacentList[vertex.voxel.label][artificialVertex.voxel.label] = Edge();
				adjacentList[vertex.voxel.label][artificialVertex.voxel.label].source = vertex.voxel.label;
				adjacentList[vertex.voxel.label][artificialVertex.voxel.label].target = artificialVertex.voxel.label;
				adjacentList[vertex.voxel.label][artificialVertex.voxel.label].weight = vertex.surface;
			}
		}
		return artificialVertex;
	}

	float getSurfaceArea(int direction) {
		float totalSurface = 0.;
		for (Vertex vertex : vertices) {
			if (vertex.isInDirection(direction)) {
				totalSurface += vertex.surface;
			}
		}
		return totalSurface;
	}

	// https://cp-algorithms.com/graph/push-relabel.html
	// active node = node != source & target, height[v] < |V| e excess[v] > 0
	float getPushRelabelMaximumFlow(int sourceDirection, int targetDirection) {
		Vertex source = this->setArtificialVertex(sourceDirection);
		Vertex target = this->setArtificialVertex(targetDirection);
		
		std::map<int, int> height;
		std::map<int, float> excess;
		std::queue<int> vqueue;	

		// Init labels
		for (Vertex vertex : vertices) {
			height[vertex.voxel.label] = 0;
			excess[vertex.voxel.label] = 0;
		}
		height[source.voxel.label] = vertices.size();
		excess[source.voxel.label] = INT_MAX;
		// Saturing arcs exiting the source
		for (auto const& edge : adjacentList[source.voxel.label]) {
			int u = edge.second.source;
			int v = edge.second.target;
			this->push(u, v, excess);
			vqueue.push(v);
		}
		
		do {
			int current = vqueue.front();			
			vqueue.pop();			
			if (current != source.voxel.label && current != target.voxel.label) {
				this->discharge(current, vqueue, height, excess);
			}
		} while (!vqueue.empty());

		float maximumFlow = 0.;
		for (auto const& edge : adjacentList[target.voxel.label]) {
			int source = edge.second.target;
			int target = edge.second.source;
			maximumFlow += adjacentList[source][target].flow;
		}
		return maximumFlow;
	}

	void discharge(int u, std::queue<int>& vqueue, std::map<int, int> &height, std::map<int, float> &excess) {
		while (excess[u] > 0) {
			bool hasPush = false;
			for (auto const& edge : adjacentList[u]) {
				int v = edge.second.target;
				if (excess[u] > 0 && edge.second.weight - edge.second.flow > 0 && height[u] == height[v] + 1) {					
					this->push(u, v, excess);
					if (excess[v] > 0) {
						vqueue.push(v);
					}
					hasPush = true;
				}
			}
			if (!hasPush)
				break;
		}
		if (excess[u] > 0) {
			this->relabel(u, height);
			vqueue.push(u);
		}
	}

	float excessFlow(int label) {
		Vertex *vertex = getVertex(label);
		float outcomingFlow = 0, incomingFlow = 0;
		for (auto const& edge : adjacentList[label])
			outcomingFlow += edge.second.flow;

		for (auto const& vertex : adjacentList) {
			int originLabel = vertex.first;
			if (originLabel == label)
				continue;
			if (adjacentList[originLabel].find(label) != adjacentList[originLabel].end())
				incomingFlow += adjacentList[originLabel][label].flow;
		}

		return outcomingFlow - incomingFlow;
	}

	bool push(int u, int v, std::map<int, float> &x) {
		float residual = adjacentList[u][v].weight - adjacentList[u][v].flow;
		float delta = (x[u] < residual) ? x[u] : residual;

		adjacentList[u][v].flow += delta;
		adjacentList[v][u].flow -= delta;
		x[u] -= delta;
		x[v] += delta;

		return delta && x[v] == delta;
	}

	void relabel(int u, std::map<int, int> &height) {
		int minLabel = INT_MAX;
		for (auto const& edge : adjacentList[u]) {			
			if (edge.second.weight - edge.second.flow > 0 && height[edge.second.target] < minLabel) {
				minLabel = height[edge.second.target];
			}
		}
		if(minLabel < INT_MAX)
			height[u] = minLabel + 1;
	}

	int getResidualCapacity(int u, int v) {
		return adjacentList[u][v].weight - adjacentList[v][u].flow;
	}

	void print() {
		for (int i = 0; i < vertices.size(); i++) {
			Vertex origin = vertices[i];
			for (int j = 0; j < vertices.size(); j++) {
				Vertex target = vertices[j];
				if (adjacentList[origin.voxel.label].find(target.voxel.label) != adjacentList[origin.voxel.label].end()) {
					printf("1  ");
				} else {
					printf("0  ");
				}
			}
			printf("\n");
		}
	}
};
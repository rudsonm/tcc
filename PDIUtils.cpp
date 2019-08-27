#include "PDIUtils.h"

SE3D PDIUtils::getSE3D(int diameter) {
	SE3D se3d;
	for (int z = 0; z < diameter; z++) {
		se3d.push_back(std::vector<std::vector<int>>());
		for (int x = 0; x < diameter; x++) {
			se3d.at(z).push_back(std::vector<int>(diameter, 0));
		}
	}
	return se3d;
}

Voxel PDIUtils::getCentroid(Voxels peak) {
	int z = 0, x = 0, y = 0;
	for (Voxel voxel : peak) {
		z += voxel.z;
		x += voxel.x;
		y += voxel.y;
	}
	z = round(z / peak.size());
	x = round(x / peak.size());
	y = round(y / peak.size());
	return Voxel(z, x, y);
}

int PDIUtils::getManhattanDistance(Voxel a, Voxel b) {
	return abs(a.z - b.z) + abs(a.x - b.x) + abs(a.y - b.y);
}

int PDIUtils::getEuclideanDistance(Voxel a, Voxel b) {
	double distance = sqrt(
		pow(a.z - b.z, 2) +
		pow(a.x - b.x, 2) +
		pow(a.y - b.y, 2)
	);
	return round(distance);
}
#pragma once
#include <vector>

struct Voxel {
	int x, y, z;
	Voxel(int z, int x, int y) : z(z), x(x), y(y) { }
};

inline bool operator <(const Voxel& a, const Voxel& b) {
	if (a.z < b.z)
		return true;
	else if (a.z == b.z && a.x < b.x)
		return true;
	else if (a.z == b.z && a.x == b.x && a.y < b.y)
		return true;
	return false;
}

using Voxels = std::vector<Voxel>;
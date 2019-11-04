#pragma once
#include <vector>

struct Voxel {
	int x, y, z, intensity = 0, label;
	Voxel() {};
	Voxel(int z, int x, int y) : z(z), x(x), y(y) { }
	Voxel(int z, int x, int y, int intensity) : z(z), x(x), y(y), intensity(intensity) { }

	bool operator <(const Voxel& b) const {
		if (intensity < b.intensity)
			return true;

		if (z < b.z)
			return true;
		else if (z == b.z && x < b.x)
			return true;
		else if (z == b.z && x == b.x && y < b.y)
			return true;
		return false;
	}

	bool operator==(const Voxel& b) const {
		return b.z == z && b.x == x && b.y == y;
	}

	void print() {
		printf("X: %i\tY: %i\tZ: %i\tI: %i\n", x, y, z, intensity);
	}
};

struct CompareVoxel {
	bool operator()(Voxel const& a, Voxel const& b)
	{
		return a.intensity < b.intensity;
	}
};

using Voxels = std::vector<Voxel>;
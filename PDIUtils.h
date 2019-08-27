#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include "Voxel.h"

using SE3D = std::vector<std::vector<std::vector<int>>>;

class PDIUtils
{
public:
	static SE3D getSE3D(int diameter);
	static Voxel getCentroid(Voxels peak);
	static int getManhattanDistance(Voxel a, Voxel b);
	static int getEuclideanDistance(Voxel a, Voxel b);
};
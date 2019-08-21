#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
using SE3D = std::vector<std::vector<std::vector<int>>>;

class PDIUtils
{
public:
	static SE3D getSE3D(int diameter);
};
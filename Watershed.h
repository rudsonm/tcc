#pragma once
#include <vector>
#include <opencv2/opencv.hpp>
#include "Voxel.h"
#include "Instance.h"
#include "UniquePriorityQueue.h"

class Watershed
{
public:
	static std::vector<cv::Mat> segmentate(const Instance& instance);
	static std::vector<cv::Mat> paint(const Instance& instance);
	static const ushort WATERSHED_BARRIER = 64477;
};


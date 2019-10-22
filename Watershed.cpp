#include "Watershed.h"

std::vector<cv::Mat> Watershed::segmentate(const Instance &instance) {
	printf("Segmentating\n");	
	std::vector<cv::Mat> result;
	for (int z = 0; z < instance.DEPTH; z++) {
		result.push_back(cv::Mat::zeros(cv::Size(instance.WIDTH, instance.HEIGHT), CV_16U));
	}
	UniquePriorityQueue<Voxel> voxels;

	for (const auto& peak : instance.peaks) {
		for (Voxel voxel : peak.second)
			result.at(voxel.z).at<ushort>(voxel.x, voxel.y) = peak.first;

		for (Voxel voxel : peak.second) {
			for (int zk = -1; zk <= 1; zk++) {
				for (int xk = -1; xk <= 1; xk++) {
					for (int yk = -1; yk <= 1; yk++) {
						if (abs(zk) + abs(xk) + abs(yk) != 1)
							continue;
						int z = voxel.z + zk,
							x = voxel.x + xk,
							y = voxel.y + yk;
						if (z < 0 || z >= instance.DEPTH
							|| x < 0 || x >= instance.HEIGHT
							|| y < 0 || y >= instance.WIDTH)
							continue;
						int neighbor = instance.rock.at(z).at<uchar>(x, y);
						if (neighbor > 0 && result.at(z).at<ushort>(x, y) == 0)
							voxels.push(Voxel(z, x, y, neighbor));
					}
				}
			}
		}
	}

	while (!voxels.empty()) {
		Voxel voxel = voxels.pop();
		std::set<int> neighborhood;
		for (int zk = -1; zk <= 1; zk++) {
			for (int xk = -1; xk <= 1; xk++) {
				for (int yk = -1; yk <= 1; yk++) {
					if (abs(zk) + abs(xk) + abs(yk) != 1)
						continue;
					int z = voxel.z + zk,
						x = voxel.x + xk,
						y = voxel.y + yk;

					if (voxel.z == z && voxel.x == x && voxel.y == y)
						continue;
					if (z < 0 || z >= instance.DEPTH
						|| x < 0 || x >= instance.HEIGHT
						|| y < 0 || y >= instance.WIDTH)
						continue;

					int neighbor = result.at(z).at<ushort>(x, y);
					int neighborDistance = instance.rock.at(z).at<uchar>(x, y);

					if (neighbor == Watershed::WATERSHED_BARRIER || neighborDistance == 0)
						continue;

					if (neighbor == 0)
						voxels.push(Voxel(z, x, y, neighborDistance));
					else
						neighborhood.insert(neighbor);
				}
			}
		}
		if (neighborhood.size() == 1)
			result.at(voxel.z).at<ushort>(voxel.x, voxel.y) = *neighborhood.begin();
		else if (neighborhood.size() > 1)
			result.at(voxel.z).at<ushort>(voxel.x, voxel.y) = Watershed::WATERSHED_BARRIER;
	}
	return result;
}

std::vector<cv::Mat> Watershed::paint(const Instance &instance) {
	std::vector<cv::Mat> result(instance.DEPTH);
	std::map<int, cv::Vec3b> colors;
	for (int z = 0; z < instance.DEPTH; z++) {
		cv::Mat colored = cv::Mat(instance.HEIGHT, instance.WIDTH, CV_8UC3);
		for (int x = 0; x < instance.HEIGHT; x++) {
			for (int y = 0; y < instance.WIDTH; y++) {
				int label = instance.rock.at(z).at<ushort>(x, y);
				cv::Vec3b color;
				if (label == 0)
					color = cv::Vec3b(0, 0, 0);
				else if (colors.find(label) == colors.end()) {
					int b = cv::theRNG().uniform(0, 256);
					int g = cv::theRNG().uniform(0, 256);
					int r = cv::theRNG().uniform(0, 256);
					color = cv::Vec3b((uchar)b, (uchar)g, (uchar)r);
					colors.insert(std::make_pair(label, color));
				} else {
					color = colors[label];
				}
				colored.at<cv::Vec3b>(x, y) = color;
			}
		}
		result.at(z) = colored;
	}
	return result;
}
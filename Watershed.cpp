#include "Watershed.h"

std::vector<cv::Mat> Watershed::segmentate(const Instance &instance) {
	printf("Segmentating\n");
	ushort WATERSHED_BARRIER = 65535;
	std::vector<cv::Mat> result;
	for (int z = 0; z < instance.DEPTH; z++) {
		result.push_back(cv::Mat::zeros(cv::Size(instance.WIDTH, instance.HEIGHT), CV_16U));
	}
	std::priority_queue<Voxel> voxels;

	for (const auto& peak : instance.peaks) {
		for (Voxel voxel : peak.second)
			result.at(voxel.z).at<ushort>(voxel.x, voxel.y) = peak.first;

		for (Voxel voxel : peak.second) {
			for (int zk = -1; zk <= 1; zk++) {
				for (int xk = -1; xk <= 1; xk++) {
					for (int yk = -1; yk <= 1; yk++) {
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

		while (!voxels.empty()) {
			Voxel voxel = voxels.top();
			voxels.pop();
			voxel.label = 0;
			for (int zk = -1; zk <= 1; zk++) {
				for (int xk = -1; xk <= 1; xk++) {
					for (int yk = -1; yk <= 1; yk++) {
						int z = voxel.z + zk,
							x = voxel.x + xk,
							y = voxel.y + yk;

						if (voxel.z == z && voxel.x == x && voxel.y == y)
							continue;
						if(z < 0 || z >= instance.DEPTH
						|| x < 0 || x >= instance.HEIGHT
						|| y < 0 || y >= instance.WIDTH)
							continue;

						int neighbor = result.at(z).at<ushort>(x, y);
						int neighborDistance = instance.rock.at(z).at<uchar>(x, y);

						if (neighbor == 0 && neighborDistance > 0)
							voxels.push(Voxel(z, x, y, neighborDistance));
						else if (voxel.label > 0 && neighbor != voxel.label)
							voxel.label = -1;
						else if (voxel.label == 0 && neighbor > 0)
							voxel.label = neighbor;
					}
				}
			}
			if (voxel.label > 0)
				result.at(voxel.z).at<ushort>(voxel.x, voxel.y) = voxel.label;			
		}
	}

	std::map<int, cv::Vec3b> colors;
	for (int z = 0; z < instance.DEPTH; z++) {
		cv::Mat colored = cv::Mat(instance.HEIGHT, instance.WIDTH, CV_8UC3);
		for (int x = 0; x < instance.HEIGHT; x++) {
			for (int y = 0; y < instance.WIDTH; y++) {
				int label = result.at(z).at<ushort>(x, y);
				cv::Vec3b color;
				if (label == 0) {
					color = cv::Vec3b(0, 0, 0);
				} else if (colors.find(label) == colors.end()) {
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
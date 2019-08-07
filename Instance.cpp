#include "Instance.h"

void Instance::reverse() {
	printf("Reversing\n");
	for (int d = 0; d < DEPTH; d++) {
		for (int x = 0; x < WIDTH; x++) {
			for (int y = 0; y < HEIGHT; y++) {
				int intensity = 255 - get(d, x, y);
				set(d, x, y, intensity);
			}
		}
	}
}

void Instance::rotate() {
	printf("Rotating\n");
	std::vector<cv::Mat> clones;
	for (int i = 0; i < DEPTH; i++)
		clones.push_back(rock.at(i).clone());

	for (int y = 0; y < HEIGHT; y++) {
		for (int z = 0; z < DEPTH; z++) {
			for (int x = 0; x < WIDTH; x++) {
				int newValue = clones.at(DEPTH - 1 - z).at<uchar>(x, y);
				set(y, x, z, newValue);
			}
		}
	}
}

cv::Mat Instance::resize(int z, double p) {
	cv::Mat resized;
	cv::resize(rock.at(z), resized, cv::Size(), p, p);
	return resized;
}

void Instance::normalize(double max) {
	printf("Normalize\n");
	uint maxValue = 0;
	for (int z = 0; z < DEPTH; z++) {
		for (int x = 0; x < WIDTH; x++) {
			for (int y = 0; y < HEIGHT; y++) {
				uint value = get(z, x, y);
				if (value > maxValue)
					maxValue = value;
			}
		}
	}
	for (int z = 0; z < DEPTH; z++) {
		for (int x = 0; x < WIDTH; x++) {
			for (int y = 0; y < HEIGHT; y++) {
				double normalizedValue = (double)get(z, x, y) / maxValue;
				set(z, x, y, normalizedValue * max);
			}
		}
	}
}

int Instance::peakCount(int z, int x, int y, int diameter) {
	std::map<int, int> valuesCount;
	int radius = diameter / 2;
	for (int zk = z - radius; zk >= z + radius; zk++) {
		for (int xk = x - radius; xk >= x + radius; xk++) {
			for (int yk = y - radius; yk >= y + radius; yk++) {
				int value = get(z - radius + zk, x - radius + xk, y - radius + yk);
				if (valuesCount.find(value) != valuesCount.end())
					valuesCount.insert(value, 1);
				else
					valuesCount.at(value)++;
			}
		}
	}
}

uint Instance::getMinNeighboring(int z, int x, int y) {		
	if (x - 1 < 0 || x + 1 >= HEIGHT ||
		y - 1 < 0 || y + 1 >= WIDTH ||
		z - 1 < 0 || z + 1 >= DEPTH)
		return 0;
	std::vector<uint> values = {
		get(z, x, y),
		get(z, x - 1, y),
		get(z, x + 1, y),
		get(z, x, y - 1),
		get(z, x, y + 1),
		get(z - 1, x, y),
		get(z + 1, x, y)
	};
	return *std::min_element(values.begin(), values.end());
}

int Instance::maximumFilter(int z, int x, int y, int diameter) {
	std::vector<Matriz> filter = std::vector<Matriz>();
	int radius = diameter / 2;
	int maxValue = 0;
	for (int zk = 0; zk < diameter; zk++) {
		for (int xk = 0; xk < diameter; xk++) {
			for (int yk = 0; yk < diameter; yk++) {
				int value = get(z - radius + zk, x - radius + xk, y - radius + yk);
				if (value > maxValue)
					maxValue = value;
			}
		}
	}
	return maxValue;
}

void Instance::distanceMap() {
	printf("Distance map\n");
	for (int z = 0; z < DEPTH; z++) {
		for (int x = 0; x < HEIGHT; x++) {
			for (int y = 0; y < WIDTH; y++) {
				if (get(z, x, y) > 0) {
					int min = getMinNeighboring(z, x, y);
					int newValue = (min == 255) ? 255 : min + 1;
					set(z, x, y, newValue);
				}
			}
		}
	}

	for (int z = DEPTH - 1; z >= 0; z--) {
		for (int x = HEIGHT - 1; x >= 0; x--) {
			for (int y = WIDTH - 1; y >= 0; y--) {
				if (get(z, x, y) > 0) {
					int min = getMinNeighboring(z, x, y);
					int newValue = (min == 255) ? 255 : min + 1;
					set(z, x, y, newValue);
				}
			}
		}
	}
}

void Instance::gaussianFilter(int kernelSize) {
	printf("Gaussian Filter\n");
	std::vector<Matriz> kernel = kernelGauss3();
	int metadeKernel = kernel.size() / 2;
	std::vector<cv::Mat> clone = clone3D();
	for (int z = metadeKernel; z < DEPTH - metadeKernel; z++) {
		for (int x = metadeKernel; x < HEIGHT - metadeKernel; x++) {
			for (int y = metadeKernel; y < WIDTH - metadeKernel; y++) {
				int somatorio = 0;
				int somatorioPesos = 0;
				for (int zk = 0; zk < kernel.size(); zk++) {
					for (int xk = 0; xk < kernel.size(); xk++) {
						for (int yk = 0; yk < kernel.size(); yk++) {
							int voxel = clone[z - metadeKernel + zk].at<uchar>(x - metadeKernel + xk, y - metadeKernel + yk);							
							somatorio += voxel * kernel[zk][xk][yk];
							somatorioPesos += kernel[zk][xk][yk];
						}
					}
				}
				set(z, x, y, somatorio / somatorioPesos);
			}
		}
	}
}

std::vector<cv::Mat> Instance::peakIdentify(int diameter) {
	std::vector<cv::Mat> clone = clone3D();
	int radius = diameter / 2;
	for (int z = radius; z < DEPTH - radius; z++) {
		for (int x = radius; x < HEIGHT - radius; x++) {
			for (int y = radius; y < WIDTH - radius; y++) {
				int maximumValue = maximumFilter(z, x, y, diameter);
				for (int zk = 0; zk < diameter; zk++) {
					for (int xk = 0; xk < diameter; xk++) {
						for (int yk = 0; yk < diameter; yk++) {
							int voxel = get(z - radius + zk, x - radius + xk, y - radius + yk);							
							if (voxel != maximumValue)
								clone.at(z - radius + zk).at<uchar>(x - radius + xk, y - radius + yk) = 0;
						}
					}
				}
			}
		}
	}
	return clone;
}

void Instance::removePeaksOnSaddles() {
	for (int z = 0; z < DEPTH; z++) {
		for (int x = 0; x < HEIGHT; x++) {
			for (int y = 0; y < WIDTH; y++) {
				int voxel = get(z, x, y);
				if (voxel == 0)
					continue;

			}
		}
	}
}
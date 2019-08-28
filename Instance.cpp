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

int Instance::getMinNeighboring(int z, int x, int y) {		
	if (x - 1 < 0 || x + 1 >= HEIGHT ||
		y - 1 < 0 || y + 1 >= WIDTH ||
		z - 1 < 0 || z + 1 >= DEPTH)
		return 0;
	std::vector<int> values = {
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
	printf("Gaussian filter\n");
	std::vector<Matriz> kernel = kernelGauss3();
	int metadeKernel = kernel.size() / 2;
	std::vector<cv::Mat> clone = clone3D();

	#pragma omp parallel for
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
	printf("Maximum filter\n");
	std::vector<cv::Mat> clone = clone3D();
	int radius = diameter / 2;

	#pragma omp parallel for
	for (int z = 0; z < DEPTH; z++) {
		for (int x = 0; x < HEIGHT; x++) {
			for (int y = 0; y < WIDTH; y++) {
				int maximumValue = maximumFilter(z, x, y, diameter);
				for (int zk = 0; zk < diameter; zk++) {
					for (int xk = 0; xk < diameter; xk++) {
						for (int yk = 0; yk < diameter; yk++) {
							int voxel = get(z - radius + zk, x - radius + xk, y - radius + yk);							
							if (voxel >= 0 && voxel != maximumValue)
								clone.at(z - radius + zk).at<uchar>(x - radius + xk, y - radius + yk) = 0;
						}
					}
				}
			}
		}
	}
	return clone;
}

std::map<int, Voxels> Instance::labeling(bool shouldColor) {
	printf("Labeling");
	int label = 1;
	std::map<int, std::set<int>> eqs;
	std::vector<std::tuple<int, int, int, int>> labeled;

	for (int z = 0; z < DEPTH; z++) {
		rock.at(z).convertTo(rock.at(z), CV_16U);
		for (int x = 0; x < HEIGHT; x++) {
			for (int y = 0; y < WIDTH; y++) {
				int voxel = rock.at(z).at<ushort>(x, y);
				if (voxel <= 0)
					continue;
				
				std::vector<int> neighborhood;
				if (z - 1 >= 0 && rock.at(z - 1).at<ushort>(x, y) > 0)
					neighborhood.push_back(rock.at(z - 1).at<ushort>(x, y));
				if (y - 1 >= 0 && rock.at(z).at<ushort>(x, y - 1) > 0)
					neighborhood.push_back(rock.at(z).at<ushort>(x, y - 1));
				if (x - 1 >= 0 && rock.at(z).at<ushort>(x - 1, y) > 0)
					neighborhood.push_back(rock.at(z).at<ushort>(x - 1, y));

				int currentLabel;
				if (neighborhood.size() == 0) {					
					currentLabel = label++;
					eqs.insert(std::make_pair(currentLabel, std::set<int>({currentLabel})));
				} else {
					currentLabel = *min_element(neighborhood.begin(), neighborhood.end());

					std::set<int> ns;
					for (int i = 0; i < neighborhood.size(); i++) {
						int neigh_label = neighborhood.at(i);
						ns.insert(
							eqs[neigh_label].begin(),
							eqs[neigh_label].end()
						);
					}

					for (int neigh_label : ns)
						eqs[neigh_label] = ns;
				}
				rock.at(z).at<ushort>(x, y) = currentLabel;
				labeled.push_back(std::make_tuple(z, x, y, currentLabel));				
			}
		}		
	}
	
	for (std::tuple<int, int, int, int> &voxel : labeled) {
		int z = std::get<0>(voxel),
			x = std::get<1>(voxel),
			y = std::get<2>(voxel);
			
		int label = rock.at(z).at<ushort>(x, y);
		int equivalence = *eqs.at(label).begin();
		std::get<3>(voxel) = equivalence;
		rock.at(z).at<ushort>(x, y) = equivalence;
		
		Voxel voxel(z, x, y);
		if (this->peaks.find(equivalence) == this->peaks.end())
			this->peaks.insert(std::make_pair(equivalence, Voxels({voxel})));
		else
			this->peaks.at(equivalence).push_back(voxel);
	}

	printf(", peaks: %d\n", peaks.size());

	if (!shouldColor)
		return peaks;

	std::vector<cv::Vec3b> colors;
	for (size_t i = 0; i < label; i++) {
		int b = cv::theRNG().uniform(0, 256);
		int g = cv::theRNG().uniform(0, 256);
		int r = cv::theRNG().uniform(0, 256);
		colors.push_back(cv::Vec3b((uchar)b, (uchar)g, (uchar)r));
	}	

	for (int z = 0; z < DEPTH; z++) {
		rock.at(z).convertTo(rock.at(z), CV_8UC1);
		cv::cvtColor(rock.at(z), rock.at(z), cv::COLOR_GRAY2BGR);
	}

	for (std::tuple<int, int, int, int> voxel : labeled) {
		int z = std::get<0>(voxel),
			x = std::get<1>(voxel),
			y = std::get<2>(voxel),
			label = std::get<3>(voxel);
		rock.at(z).at<cv::Vec3b>(x, y) = colors.at(label);
	}

	return peaks;
}

std::map<int, Voxels> Instance::removePeaksOnSaddles() {
	printf("Removing peaks on saddles");
	std::vector<int> falsePeaks;	
	for (auto const& peak : peaks) {
		int label = peak.first;
		std::set<Voxel> dilated;
		dilated.insert(peak.second.begin(), peak.second.end());
		bool falsePeak = false;
		int dilatedSize;
		do {
			Voxels neighborhood;
			for (Voxel voxel : dilated) {
				int distanceValue = get(voxel.z, voxel.x, voxel.y);
				for (int zk = -1; zk <= 1; zk++)
					for(int xk = -1; xk <= 1; xk++)
						for(int yk = -1; yk <= 1; yk++) {
							Voxel neighVoxel(voxel.z + zk, voxel.x + xk, voxel.y + yk);
							int neighDistanceValue = get(neighVoxel);
							if (neighDistanceValue == distanceValue)
								neighborhood.push_back(neighVoxel);
							else if (neighDistanceValue > distanceValue)
								falsePeak = true;
						}
			}			
			dilatedSize = dilated.size();
			dilated.insert(neighborhood.begin(), neighborhood.end());
		} while (dilated.size() > dilatedSize && !falsePeak);
		if(falsePeak)
			falsePeaks.push_back(label);
	}
	printf(", falses: %d\n", falsePeaks.size());
	for (int falsePeak : falsePeaks)
		peaks.erase(falsePeak);
	return peaks;
}

std::map<int, Voxels> Instance::mergePeaks() {
	printf("Merging peaks");
	int **matrix = new int*[peaks.size()];
	for (int i = 0; i < peaks.size(); i++)
		matrix[i] = new int[peaks.size()];
	
	std::vector<Voxel> centroids;
	std::vector<int> labels;
	std::vector<int> distances;
	for (const auto& peak : this->peaks) {
		labels.push_back(peak.first);
		centroids.push_back(PDIUtils::getCentroid(peak.second));
		Voxel voxel = peak.second.at(0);
		distances.push_back(get(voxel));
	}
	
	for (int i = 0; i < centroids.size(); i++)
		for (int j = i; j < centroids.size(); j++)
			matrix[i][j] = PDIUtils::getEuclideanDistance(centroids[i], centroids[j]);
	
	std::vector<int> merged;
	for (int i = 0; i < labels.size(); i++) {
		for (int j = i + 1; j < labels.size(); j++) {
			if (matrix[i][j] < distances[i]) {
				int closestToSolid = (distances[i] < distances[j]) ? labels[i] : labels[j];
				merged.push_back(closestToSolid);
			}
		}
	}
	printf(", merged: %d\n", merged.size());
	for (int label : merged)
		peaks.erase(label);
	return peaks;
}
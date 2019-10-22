#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <cmath>
#include <map>
#include <tuple>
#include <time.h>
#include "Voxel.h"
#include <omp.h>
#include "PDIUtils.h"

# define M_PI           3.14159265358979323846

using uint = unsigned int;
using Matriz = std::vector<std::vector<double>>;

class Instance
{
public:
	std::vector<cv::Mat> rock;
	std::map<int, Voxels> peaks;
	int HEIGHT;
	int WIDTH;
	int DEPTH;
	long double porosity;

	void rotate();
	void reverse();	
	void normalize(double max);	
	cv::Mat resize(int z, double p);
	int getMinNeighboring(int x, int y, int z);
	int maximumFilter(int z, int x, int y, int diameter);
	std::set<ushort> getNeighborsLabels(int z, int x, int y);

	void distanceMap();
	void gaussianFilter(int kernelSize);
	std::vector<cv::Mat> peakIdentify(int radius);
	std::map<int, Voxels> labeling(bool shouldColor);
	std::map<int, Voxels> removePeaksOnSaddles();
	std::map<int, Voxels> mergePeaks();
	std::vector<cv::Mat> watershed();

	void changeFormat(const int OPEN_CV_FORMAT) {
		for (int z = 0; z < DEPTH; z++)
			rock.at(z).convertTo(rock.at(z), OPEN_CV_FORMAT);
	}

	int get(int z, int x, int y) {
		if (z < 0 || z >= DEPTH || x < 0 || x >= HEIGHT || y < 0 || y >= WIDTH)
			return -1;
		return (int) this->rock.at(z).at<uchar>(x, y);
	}

	int get(Voxel voxel) {
		return get(voxel.z, voxel.x, voxel.y);
	}

	void set(int z, int x, int y, int value) {
		this->rock.at(z).at<uchar>(x, y) = value;
	}	

	void show(std::string windowTitle, int z) {
		cv::imshow(windowTitle, this->rock.at(z).clone());
	}

	void show(std::string windowTitle, int z, int p) {
		cv::imshow(windowTitle, resize(z, p));
	}

	std::vector<std::vector<std::vector<double>>> gaussianKernel(int size, double sigma = 0.35) {
		std::vector<std::vector<std::vector<double>>> kernel(size, std::vector<std::vector<double>>(size, std::vector<double>(size)));
		for (double z = 0; z < size; z++) {
			for (double x = 0; x < size; x++) {
				for (double y = 0; y < size; y++) {
					kernel[x][y][z] = std::exp(
						-(double)(
							std::pow(x - size / 2, 2)
							+ std::pow(y - size / 2, 2)
							+ std::pow(z - size / 2, 2)
							) / 2. * std::pow(sigma, 2)
					);
					kernel[x][y][z] *= 1. / std::pow(std::sqrt(2. * M_PI) * sigma, 3);
				}
			}
		}
		return kernel;
	}

	std::vector<cv::Mat> clone3D() {
		std::vector<cv::Mat> clone = std::vector<cv::Mat>();
		for (int z = 0; z < DEPTH; z++) 
			clone.push_back(rock[z].clone());
		return clone;
	}

	void updatePeaks(std::map<int, Voxels> peaks) {
		this->peaks = peaks;
		for (int z = 0; z < DEPTH; z++)
			rock.at(z) = cv::Mat::zeros(cv::Size(WIDTH, HEIGHT), CV_8UC3);

		for (auto const& peak : peaks) {
			cv::Vec3b color(
				cv::theRNG().uniform(0, 256),
				cv::theRNG().uniform(0, 256),
				cv::theRNG().uniform(0, 256)
			);
			for(Voxel voxel : peak.second)
				rock.at(voxel.z).at<cv::Vec3b>(voxel.x, voxel.y) = color;
		}
	}

	Instance(const std::vector<cv::Mat> &rock) {		
		for (int z = 0; z < rock.size(); z++)
			this->rock.push_back(rock.at(z).clone());
		this->DEPTH = rock.size();
		this->HEIGHT = rock.at(0).rows;
		this->WIDTH = rock.at(0).cols;
	}

	Instance(const Instance& instance) {
		for (int z = 0; z < rock.size(); z++)
			this->rock.push_back(instance.rock.at(z).clone());
		this->DEPTH = instance.DEPTH;
		this->HEIGHT = instance.HEIGHT;
		this->WIDTH = instance.WIDTH;
		this->porosity = instance.porosity;
	}

	Instance(std::string image, int HEIGHT, int WIDTH, int DEPTH) {
		this->HEIGHT = HEIGHT;
		this->WIDTH = WIDTH;
		this->DEPTH = DEPTH;

		std::ifstream rawImage("images/" + image);

		if (!rawImage) {
			printf("Image is not available \n");
			exit(1);
		}

		printf("Reading...\n");
		this->porosity = 0.;
		for (int d = 0; d < DEPTH; d++) {
			rock.push_back(cv::Mat(HEIGHT, WIDTH, CV_8UC1));			
			for (int x = 0; x < HEIGHT; x++) {
				for (int y = 0; y < WIDTH; y++) {
					//rawImage >> rock.at(d).at<uchar>(x, y);
					rawImage >> rock.at(d).at<uchar>(x, y);
					rock.at(d).at<uchar>(x, y) = (rock.at(d).at<uchar>(x, y) - 48) * 255;
										
					if ((int) rock.at(d).at<uchar>(x, y) == 0)
						this->porosity++;
				}
			}
			if((d + 1) % 100 == 0)
				printf("%d/%d\n", d + 1, DEPTH);
		}
		this->porosity = this->porosity / (double) (this->DEPTH * this->HEIGHT * this->WIDTH);

		rawImage.close();
	}
};
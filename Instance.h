#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <math.h>
#include <map>

using uint = unsigned int;
using Matriz = std::vector<std::vector<int>>;
class Instance
{
public:
	std::vector<cv::Mat> rock;
	int HEIGHT;
	int WIDTH;
	int DEPTH;
	long double porosity;

	void rotate();
	void reverse();	
	void normalize(double max);	
	cv::Mat resize(int z, double p);
	uint getMinNeighboring(int x, int y, int z);
	int maximumFilter(int z, int x, int y, int diameter);
	int peakCount(int z, int x, int y, int diameter);

	void distanceMap();
	void gaussianFilter(int kernelSize);
	std::vector<cv::Mat> peakIdentify(int radius);
	void removePeaksOnSaddles();

	uint get(int z, int x, int y) {
		return (int) this->rock.at(z).at<uchar>(x, y);
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

	std::vector<Matriz> kernelGauss3() {
		std::vector<Matriz> novoKernelGauss;
		novoKernelGauss.push_back(Matriz());
		novoKernelGauss.at(0).push_back({ 1, 2, 1 });
		novoKernelGauss.at(0).push_back({ 2, 2, 2 });
		novoKernelGauss.at(0).push_back({ 1, 2, 1 });
		novoKernelGauss.push_back(Matriz());
		novoKernelGauss.at(1).push_back({ 1, 2, 1 });
		novoKernelGauss.at(1).push_back({ 2, 4, 2 });
		novoKernelGauss.at(1).push_back({ 1, 2, 1 });
		novoKernelGauss.push_back(Matriz());
		novoKernelGauss.at(2).push_back({ 1, 2, 1 });
		novoKernelGauss.at(2).push_back({ 2, 2, 2 });
		novoKernelGauss.at(2).push_back({ 1, 2, 1 });
		return novoKernelGauss;
	}

	Matriz kernelGauss5() {
		Matriz novoKernelGauss = Matriz();
		novoKernelGauss.push_back({ 1,  4,  6, 4,  1 });
		novoKernelGauss.push_back({ 4, 16, 24, 16, 4 });
		novoKernelGauss.push_back({ 6, 24, 36, 24, 6 });
		novoKernelGauss.push_back({ 4, 16, 24, 16, 4 });
		novoKernelGauss.push_back({ 1,  4,  6, 4,  1 });
		return novoKernelGauss;
	}

	std::vector<cv::Mat> clone3D() {
		std::vector<cv::Mat> clone = std::vector<cv::Mat>();
		for (int z = 0; z < DEPTH; z++) 
			clone.push_back(rock[z].clone());
		return clone;
	}

	Instance(const std::vector<cv::Mat> &rock) {
		this->rock = rock;
		this->DEPTH = rock.size();
		this->HEIGHT = rock.at(0).rows;
		this->WIDTH = rock.at(0).cols;
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
			rock.push_back(cv::Mat(HEIGHT, WIDTH, CV_8UC1, cv::Scalar(0)));			
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
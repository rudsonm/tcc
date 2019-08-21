#include "PDIUtils.h"

SE3D PDIUtils::getSE3D(int diameter) {
	SE3D se3d;
	for (int z = 0; z < diameter; z++) {
		se3d.push_back(std::vector<std::vector<int>>());
		for (int x = 0; x < diameter; x++) {
			se3d.at(z).push_back(std::vector<int>(diameter, 0));
		}
	}
	return se3d;
}

//void PDIUtils::writeNewInstance() {
//	ofstream saida("LV60A_amostra_50.dat");
//	for (int z = 0; z < 50; z++) {
//		for (int x = 0; x < 50; x++) {
//			for (int y = 0; y < 50; y++) {
//				int voxel = instance.rock.at(z).at<char>(x, y);
//				saida << (voxel == 255) ? 1 : 0;
//			}
//		}
//	}
//	saida.close();
//	return 0;
//}
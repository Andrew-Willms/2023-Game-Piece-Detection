#pragma once

#include <opencv2/imgproc.hpp>

using namespace std;
using namespace cv;



inline void SquareDilate(InputArray source, OutputArray destination, int kernelSize) {

	if (kernelSize % 2 == 0) {
		kernelSize++;
	}

	const Mat dilationKernel = getStructuringElement(MORPH_RECT, Size(kernelSize, kernelSize));
	dilate(source, destination, dilationKernel);
}

inline void SquareErode(InputArray source, OutputArray destination, int kernelSize) {

	if (kernelSize % 2 == 0) {
		kernelSize++;
	}

	const Mat erosionKernel = getStructuringElement(MORPH_RECT, Size(kernelSize, kernelSize));
	cv::erode(source, destination, erosionKernel);
}

inline void DrawContour(Mat& image, const vector<Point>& contour, const Scalar& color = Scalar(0, 0, 0), const int thickness = 1) {

	vector<vector<Point>> dummy = vector<vector<Point>>();

	dummy.push_back(contour);

	drawContours(image, dummy, 0, color, thickness);
}
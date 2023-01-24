#pragma once

#include <opencv2/imgproc.hpp>
#include <opencv2/core/types.hpp>

using namespace std;
using namespace cv;



inline void DrawContour(Mat& image, const vector<Point>& contour, const Scalar& color = Scalar(0, 0, 0), const int thickness = 1) {

	vector<vector<Point>> dummy = vector<vector<Point>>();

	dummy.push_back(contour);

	drawContours(image, dummy, 0, color, thickness);
}

inline Point ContourCentroid(const vector<Point>& contour) {

	const Moments moment = moments(contour, true);
	return Point(moment.m10 / moment.m00, moment.m01 / moment.m00);
}

inline vector<vector<Point>> FilteredContours(const vector<vector<Point>>& contours, const int minArea, const int maxArea) {

	vector<vector<Point>> filteredContours;

	for (const vector<Point>& contour : contours) {

		const double area = contourArea(contour);

		if (area < minArea || area > maxArea) {
			continue;
		}

		filteredContours.push_back(contour);
	}

	return filteredContours;
}

inline const vector<Point>* SmallestContour(const vector<vector<Point>>& contours) {

	if (contours.empty()) {
		return nullptr;
	}

	const vector<Point>* smallestContour = contours.data();
	double smallestArea = contourArea(contours[0]);

	for (const vector<Point>& contour : contours) {

		const double currentArea = contourArea(contour);

		if (currentArea < smallestArea) {
			smallestArea = currentArea;
			smallestContour = &contour;
		}
	}

	return smallestContour;
}
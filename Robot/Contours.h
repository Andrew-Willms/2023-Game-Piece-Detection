#pragma once

#include <opencv2/imgproc.hpp>
#include <opencv2/core/types.hpp>

#include "Points.h"

using namespace std;
using namespace cv;



inline void DrawContour(Mat& image, const vector<Point>& contour, const Scalar& color = Scalar(0, 0, 0), const int thickness = 1) {

	vector<vector<Point2i>> singleContour = vector<vector<Point2i>>();
	singleContour.push_back(contour);

	drawContours(image, singleContour, 0, color, thickness);
}

inline Point2i ContourCentroid(const vector<Point2i>& contour) {

	const Moments moment = moments(contour, true);
	return Point2i(moment.m10 / moment.m00, moment.m01 / moment.m00);
}

inline vector<vector<Point2i>> FilteredContours(const vector<vector<Point2i>>& contours, const int minArea, const int maxArea) {

	vector<vector<Point2i>> filteredContours;

	for (const vector<Point2i>& contour : contours) {

		const double area = contourArea(contour);

		if (area < minArea || area > maxArea) {
			continue;
		}

		filteredContours.push_back(contour);
	}

	return filteredContours;
}

inline const vector<Point2i>* SmallestContour(const vector<vector<Point2i>>& contours) {

	if (contours.empty()) {
		return nullptr;
	}

	const vector<Point2i>* smallestContour = contours.data();
	double smallestArea = contourArea(contours[0]);

	for (const vector<Point2i>& contour : contours) {

		const double currentArea = contourArea(contour);

		if (currentArea < smallestArea) {
			smallestArea = currentArea;
			smallestContour = &contour;
		}
	}

	return smallestContour;
}

inline const vector<Point2i>* BiggestContour(const vector<vector<Point2i>>& contours) {

	if (contours.empty()) {
		return nullptr;
	}

	const vector<Point2i>* biggestContour = contours.data();
	double biggestArea = contourArea(contours[0]);

	for (const vector<Point2i>& contour : contours) {

		const double currentArea = contourArea(contour);

		if (currentArea > biggestArea) {
			biggestArea = currentArea;
			biggestContour = &contour;
		}
	}

	return biggestContour;
}

inline const vector<Point2i>* MostCentralContour(const vector<vector<Point2i>>& contours, const Point2i cameraResolution) {

	if (contours.empty()) {
		return nullptr;
	}

	const Point2i centerPoint = Point2i(cameraResolution.x / 2, cameraResolution.y / 2);

	const vector<Point2i>* mostCentralContour = contours.data();
	double smallestDistanceToCenter = DistanceBetweenPoints(ContourCentroid(*contours.data()), centerPoint);

	for (const vector<Point2i>& contour : contours) {

		const Point2i currentCentroid = ContourCentroid(contour);
		const double currentDistanceToCenter = DistanceBetweenPoints(currentCentroid, centerPoint);

		if (currentDistanceToCenter < smallestDistanceToCenter) {
			smallestDistanceToCenter = currentDistanceToCenter;
			mostCentralContour = &contour;
		}
	}

	return mostCentralContour;
}
#pragma once

#include <opencv2/core/types.hpp>

using namespace cv;
using namespace std;



inline double DistanceBetweenPoints(const Point2i a, const Point2i b) {

	const int deltaX = a.x - b.x;
	const int deltaY = a.y - b.y;

	const int deltaXSquared = deltaX * deltaX;
	const int deltaYSquared = deltaY * deltaY;

	const double squareRoot = sqrt(deltaXSquared + deltaYSquared);

	return squareRoot;

	// don't use this version cause for some reason it sometimes returns NAN
	//return sqrt(( (a.x - b.x) * (a.x - b.x)) + ((a.y - b.y) * (a.y - b.x)));
}

inline Point FarthestPoint(const vector<Point2i>& points, const Point2i basePoint) {

	Point2i farthestPoint = points[0];
	double farthestDistance = DistanceBetweenPoints(basePoint, points[0]);

	for (const Point2i point : points) {

		const double currentDistance = DistanceBetweenPoints(basePoint, point);

		if (currentDistance > farthestDistance) {
			farthestDistance = currentDistance;
			farthestPoint = point;
		}
	}

	return farthestPoint;
}

inline Point2i AveragePointInGroup(const vector<Point2i>& group) {

	double sumX = 0, sumY = 0;

	for (const Point2i point : group) {

		sumX += point.x;
		sumY += point.y;
	}

	return Point2i(sumX / group.size(), sumY / group.size());
}
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <cmath>
#include <chrono>
#include <iostream>
#include <string>
#include "Colors.h"
#include "Contours.h"
#include "Parameters.h"
#include "Wrappers.h"
#include "MultiImageWindow.h"

using namespace cv;
using namespace std;
using namespace std::chrono;



double PointDistance(const Point a, const Point b) {

	const int deltaX = a.x - b.x;
	const int deltaY = a.y - b.y;

	const int deltaXSquared = deltaX * deltaX;
	const int deltaYSquared = deltaY * deltaY;

	const double squareRoot = sqrt(deltaXSquared + deltaYSquared);

	return squareRoot;

	// don't use this version cause for some reason it sometimes returns NAN
	//return sqrt(( (a.x - b.x) * (a.x - b.x)) + ((a.y - b.y) * (a.y - b.x)));
}

Point FarthestPoint(const vector<Point>& points, const Point basePoint) {

	Point farthestPoint = points[0];
	double farthestDistance = PointDistance(basePoint, points[0]);

	for (const Point point : points) {

		const double currentDistance = PointDistance(basePoint, point);

		if (currentDistance > farthestDistance) {
			farthestDistance = currentDistance;
			farthestPoint = point;
		}
	}

	return farthestPoint;
}

int FarthestPointIndex(const vector<Point>& points, const Point basePoint) {

	int farthestPointIndex = 0;
	double farthestDistance = PointDistance(basePoint, points[0]);

	for (int i = 0; i < points.size(); i++) {

		const double currentDistance = PointDistance(basePoint, points[i]);

		if (currentDistance > farthestDistance) {
			farthestDistance = currentDistance;
			farthestPointIndex = i;
		}
	}

	return farthestPointIndex;
}

Point FarthestAveragePoint(Mat& image, const vector<Point>& points, const Point basePoint, 
	const int searchRange = 30, const double distanceTolerance = 0.90, const double maxDistanceFromFarthest = 40) {

	const Point farthestPoint = FarthestPoint(points, basePoint);
	const int farthestPointIndex = FarthestPointIndex(points, basePoint);
	const double farthestPointDistance = PointDistance(points[farthestPointIndex], basePoint);

	vector<Point> farPoints = vector<Point>();
	farPoints.push_back(points[farthestPointIndex]);

	for (int i = 1; i < searchRange; i++) {

		if (farthestPointIndex + i >= points.size()) {
			continue;
		}

		if (PointDistance(points[farthestPointIndex + i], basePoint) < farthestPointDistance * distanceTolerance) {
			continue;
		}

		if (PointDistance(farthestPoint, points[farthestPointIndex + i]) > maxDistanceFromFarthest) {
			continue;
		}

		farPoints.push_back(points[farthestPointIndex + i]);
	}


	for (int i = 1; i < searchRange; i++) {

		if (farthestPointIndex - i < 0) {
			continue;
		}

		if (PointDistance(points[farthestPointIndex - i], basePoint) < farthestPointDistance * distanceTolerance) {
			continue;
		}

		if (PointDistance(farthestPoint, points[farthestPointIndex - i]) > maxDistanceFromFarthest) {
			continue;
		}

		farPoints.push_back(points[farthestPointIndex - i]);
	}

	int totalX = 0;
	int totalY = 0;

	for (const Point farPoint : farPoints) {
		totalX += farPoint.x;
		totalY += farPoint.y;

		circle(image, farPoint, 1, BLUE);
	}

	return Point(totalX / farPoints.size(), totalY / farPoints.size());
}



double LineAngleFromVertical(const Point center, const Point endpoint) {

	const int deltaX = center.x - endpoint.x;
	const int deltaY = center.y - endpoint.y;

	return atan2(deltaY, deltaX);
}



void FindFilterDrawContours(const Mat& sourceImage, Mat& targetImage, const int minContourArea, const int maxContourArea) {

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;

	findContours(sourceImage, contours, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE);

	const vector<vector<Point>> filteredContours = FilteredContours(contours, minContourArea, maxContourArea);

	const vector<Point>* smallestContour = SmallestContour(filteredContours);

	if (smallestContour == nullptr) {
		return;
	}

	DrawContour(targetImage, *smallestContour, MAGENTA);

	const Point centroid = ContourCentroid(*smallestContour);
	const Point farthestAveragePoint = FarthestAveragePoint(targetImage, *smallestContour, centroid);
	const Point farthestPoint = FarthestPoint(*smallestContour, centroid);

	const double angle = LineAngleFromVertical(centroid, farthestPoint);

	line(targetImage, farthestAveragePoint, centroid, GREEN);
	line(targetImage, farthestPoint, centroid, RED);

	putText(targetImage, to_string(angle), centroid, 0, 1.0, BLUE);
}



void CeilingToOdd(int& number) {

	if (number % 2 == 0) {
		number++;
	}
}



int main() {

	constexpr int cameraId = 0;
	VideoCapture videoCapture(cameraId);
	Mat image, imageHsv, mask, maskDilated, maskEroded, blurred, edges, contoursDilated, contoursEroded, contours;

	Scalar lowerColorLimit, upperColorLimit;
	Mat dilationKernel, erosionKernel;

	Parameters parameters = Parameters();
	parameters.CreateTrackbars();

	while (true) {

		time_point<steady_clock> startTime = high_resolution_clock::now();

		CeilingToOdd(parameters.BlurKernelSize);
		CeilingToOdd(parameters.ContourDilation);
		CeilingToOdd(parameters.ContourErosion);
		lowerColorLimit = Scalar(parameters.HueMin, parameters.SaturationMin, parameters.ValueMin);
		upperColorLimit = Scalar(parameters.HueMax, parameters.SaturationMax, parameters.ValueMax);

		videoCapture.read(image);
		contours = image.clone();

		cvtColor(image, imageHsv, COLOR_BGR2HSV);
		inRange(imageHsv, lowerColorLimit, upperColorLimit, mask);
		 
		SquareErode(mask, maskEroded, parameters.MaskErosion);
		SquareDilate(maskEroded, maskDilated, parameters.MaskDilation);

		GaussianBlur(maskDilated, blurred, Size(parameters.BlurKernelSize, parameters.BlurKernelSize), parameters.BlurSigmaX, parameters.BlurSigmaY);
		Canny(blurred, edges, parameters.CannyThreshold1, parameters.CannyThreshold2);
		SquareDilate(edges, contoursDilated, parameters.ContourDilation);
		SquareErode(contoursDilated, contoursEroded, parameters.ContourErosion);
		FindFilterDrawContours(contoursEroded, contours, parameters.MinContourArea, parameters.MaxContourArea);

		MultiImageWindow multiImageWindow = MultiImageWindow("Window", 1920, 980, 4, 2);

		multiImageWindow.AddImage(mask, 0, 0, "Mask");
		multiImageWindow.AddImage(maskEroded, 1, 0, "Mask Eroded");
		multiImageWindow.AddImage(maskDilated, 2, 0, "Mask Dilated");
		multiImageWindow.AddImage(blurred, 3, 0, "Blur");
		multiImageWindow.AddImage(edges, 0, 1, "Canny");
		multiImageWindow.AddImage(contoursDilated, 1, 1, "Dilated");
		multiImageWindow.AddImage(contoursEroded, 2, 1, "Eroded");
		multiImageWindow.AddImage(contours, 3, 1, "Contours");
		multiImageWindow.Show();

		time_point<steady_clock> endTime = high_resolution_clock::now();
		duration<double, milli> duration = endTime - startTime;
		cout << duration.count() << endl;

		waitKey(1);
	}
}
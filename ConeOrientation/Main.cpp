#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <cmath>
#include "Colors.h"
#include "Parameters.h"
#include "Wrappers.h"
#include "MultiImageWindow.h"

using namespace cv;
using namespace std;



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

Point ContourCentroid(const vector<Point>& contour) {

	const Moments moment = moments(contour, true);
	return Point( moment.m10 / moment.m00, moment.m01 / moment.m00);
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
	const int searchRange = 30, const double distanceTolerance = 0.90, const double maxDistanceFromFarthest = 30) {

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



vector<vector<Point>> FilteredContours(const vector<vector<Point>>& contours, const int minArea, const int maxArea) {

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

const vector<Point>* SmallestContour(const vector<vector<Point>>& contours) {

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

	constexpr int cameraId = 2;
	VideoCapture videoCapture(cameraId);
	Mat image, mask, imageHsv, imageGray, imageBlur, imageCanny, imageDilated, imageEroded, imageContours;

	Scalar lowerColorLimit, upperColorLimit;
	Mat dilationKernel, erosionKernel;

	Parameters parameters = Parameters();
	parameters.CreateTrackbars();

	while (true) {

		CeilingToOdd(parameters.BlurKernelSize);
		CeilingToOdd(parameters.DilationKernelSize);
		CeilingToOdd(parameters.ErosionKernelSize);

		videoCapture.read(image);
		imageContours = image.clone();

		lowerColorLimit = Scalar(parameters.HueMin, parameters.SaturationMin, parameters.ValueMin);
		upperColorLimit = Scalar(parameters.HueMax, parameters.SaturationMax, parameters.ValueMax);
		cvtColor(image, imageHsv, COLOR_BGR2HSV);
		inRange(imageHsv, lowerColorLimit, upperColorLimit, mask);

		GaussianBlur(mask, imageBlur, Size(parameters.BlurKernelSize, parameters.BlurKernelSize), parameters.BlurSigmaX, parameters.BlurSigmaY);
		Canny(imageBlur, imageCanny, parameters.CannyThreshold1, parameters.CannyThreshold2);
		SquareDilate(imageCanny, imageDilated, parameters.DilationKernelSize);
		SquareErode(imageDilated, imageEroded, parameters.ErosionKernelSize);
		FindFilterDrawContours(imageEroded, imageContours, parameters.MinContourArea, parameters.MaxContourArea);

		MultiImageWindow multiImageWindow = MultiImageWindow("Window", 1800, 900, 3, 2);

		multiImageWindow.AddImage(mask, 0, 0, "Mask");
		multiImageWindow.AddImage(imageBlur, 1, 0, "Blur");
		multiImageWindow.AddImage(imageCanny, 2, 0, "Canny");
		multiImageWindow.AddImage(imageDilated, 0, 1, "Dilated");
		multiImageWindow.AddImage(imageEroded, 1, 1, "Eroded");
		multiImageWindow.AddImage(imageContours, 2, 1, "Contours");
		multiImageWindow.Show();

		waitKey(1);
	}
}
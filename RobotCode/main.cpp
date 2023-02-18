#include <chrono>
#include <thread>
#include <fmt/format.h>
#include <networktables/NetworkTableInstance.h>
#include <networktables/NetworkTable.h>
#include <networktables/DoubleTopic.h>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <cmath>
#include <iostream>
#include <string>
#include "Colors.h"
#include "Contours.h"
#include "Parameters.h"
#include "Wrappers.h"
#include "MultiImageWindow.h"
#include "Trigonometry.h"

using namespace cv;
using namespace std;
using namespace std::chrono;

double angle = 0;
double xPosition = 0;
double yPosition = 0;

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
	//const Point farthestAveragePoint = FarthestAveragePoint(targetImage, *smallestContour, centroid);
	const Point farthestPoint = FarthestPoint(*smallestContour, centroid);

	//line(targetImage, farthestAveragePoint, centroid, GREEN);
	line(targetImage, farthestPoint, centroid, RED);

	const Point2i cameraResolution = Point2i(640, 480);
	const Point2d cameraFov = Point2d(53.0l / 180.0l * PI, 38.0l / 180.0l * PI);
	const Point3d cameraOffset = Point3d(3, 15, 39);
	const Point2d cameraAngle = Point2d(0.0l / 180.0l * PI, -60.0l / 180.01 * PI);

	const Point2d centroidPosition = CalculateObjectDisplacement(centroid, cameraResolution, cameraFov, cameraOffset, cameraAngle);
	const Point2d tipPosition = CalculateObjectDisplacement(farthestPoint, cameraResolution, cameraFov, cameraOffset, cameraAngle);
	const Point2d midPointPosition = CalculateConeMidpoint(centroidPosition, tipPosition);
	const double coneAngle = CalculateConeAngle(centroidPosition, tipPosition);

	xPosition = centroidPosition.x;
	yPosition = centroidPosition.y;
	angle = coneAngle;

	//const string text = "X:" + to_string(midPointPosition.x) + ", Y:" + to_string(midPointPosition.y) + " A:" + to_string(coneAngle * 180 / PI);
	//putText(targetImage, text, Point2i(20, 45), 0, 0.75, BLUE);
}



void CeilingToOdd(int& number) {

	if (number % 2 == 0) {
		number++;
	}
}



int main() {

	constexpr int cameraId = 0;
	VideoCapture videoCapture(cameraId);

	if (!videoCapture.isOpened()) {
		fmt::print("The video capture was not opened");
		return 0;
	}

	Mat image, imageHsv, mask, maskDilated, maskEroded, blurred, edges, contoursDilated, contoursEroded, contours;

	Scalar lowerColorLimit, upperColorLimit;
	Mat dilationKernel, erosionKernel;

	Parameters parameters = Parameters();
	//parameters.CreateTrackbars();
	
	nt::DoublePublisher dblPubAngle, dblPubX, dblPubY;
	auto inst = nt::NetworkTableInstance::GetDefault();
	auto table = inst.GetTable("rpi");
	//auto pubOp = new PubSubOption();
	inst.StartClient4("example client");
	inst.SetServerTeam(4678);  // where TEAM=190, 294, etc, or use inst.setServer("hostname") or similar
	inst.StartDSClient();  // recommended if running on DS computer; this gets the robot IP from the DS
	dblPubAngle = table->GetDoubleTopic("tv_angle").Publish();
	dblPubX = table->GetDoubleTopic("tv_x").Publish();
	dblPubY = table->GetDoubleTopic("tv_y").Publish();
	int cnt = 0;
	fmt::print("hello");

	while (true) {

		time_point<system_clock> startTime = high_resolution_clock::now();

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

		//time_point<steady_clock> startTime = high_resolution_clock::now();

		FindFilterDrawContours(contoursEroded, contours, parameters.MinContourArea, parameters.MaxContourArea);

		//MultiImageWindow multiImageWindow = MultiImageWindow("Window", 1920, 980, 4, 2);
		//multiImageWindow.AddImage(mask, 0, 0, "Mask");
		//multiImageWindow.AddImage(maskEroded, 1, 0, "Mask Eroded");
		//multiImageWindow.AddImage(maskDilated, 2, 0, "Mask Dilated");
		//multiImageWindow.AddImage(blurred, 3, 0, "Blur");
		//multiImageWindow.AddImage(edges, 0, 1, "Canny");
		//multiImageWindow.AddImage(contoursDilated, 1, 1, "Dilated");
		//multiImageWindow.AddImage(contoursEroded, 2, 1, "Eroded");
		//multiImageWindow.AddImage(contours, 3, 1, "Contours");
		//multiImageWindow.Show();

		time_point<system_clock> endTime = high_resolution_clock::now();
		duration<double, milli> duration = endTime - startTime;

		dblPubAngle.Set((angle * 180l / 3.14159l));
		dblPubX.Set(xPosition);
		dblPubY.Set(yPosition);
		fmt::print("t: " + to_string(duration.count()) + ", x: " + to_string(xPosition) + ", y: " + to_string(yPosition) + ", angle: " + to_string(angle * 180l / 3.14159l) + "\n");
		waitKey(1);
	}
}
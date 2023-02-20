#include <fmt/format.h>
#include <networktables/NetworkTableInstance.h>
#include <networktables/NetworkTable.h>
#include <networktables/DoubleTopic.h>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#include <chrono>
#include <thread>
#include <cmath>
#include <iostream>
#include <string>

#include "ConeDetails.h"
#include "Contours.h"
#include "Parameters.h"
#include "Wrappers.h"
#include "Trigonometry.h"
#include "Points.h"

#define PRINT_DATA true;

using namespace cv;
using namespace std;
using namespace std::chrono;



#ifdef PRINT_DATA
string FixLength(const string& text, const char length) {

	int textLength = text.length();

	if (textLength > length) {
		return text.substr(0, length);
	}

	const string spaces =
		"                                                                "
		"                                                                "
		"                                                                "
		"                                                                ";

	return text + spaces.substr(0, length - textLength);
}
#endif

void CeilingToOdd(int& number) {

	if (number % 2 == 0) {
		number++;
	}
}

void PreProcessImage(const Mat& sourceImage, Mat& targetImage, Parameters parameters) {

	Mat imageHsv, mask, maskDilated, maskEroded, blurred, edges, contoursDilated, contoursEroded;

	Scalar lowerColorLimit = Scalar(parameters.HueMin, parameters.SaturationMin, parameters.ValueMin);
	Scalar upperColorLimit = Scalar(parameters.HueMax, parameters.SaturationMax, parameters.ValueMax);

	cvtColor(sourceImage, imageHsv, COLOR_BGR2HSV);
	inRange(imageHsv, lowerColorLimit, upperColorLimit, mask);

	SquareErode(mask, maskEroded, parameters.MaskErosion);
	SquareDilate(maskEroded, maskDilated, parameters.MaskDilation);

	GaussianBlur(maskDilated, blurred, Size(parameters.BlurKernelSize, parameters.BlurKernelSize), parameters.BlurSigmaX, parameters.BlurSigmaY);
	Canny(blurred, edges, parameters.CannyThreshold1, parameters.CannyThreshold2);

	SquareDilate(edges, contoursDilated, parameters.ContourDilation);
	SquareErode(contoursDilated, contoursEroded, parameters.ContourErosion);

	copyMakeBorder(contoursEroded, targetImage, 1, 1, 1, 1, BORDER_CONSTANT);
}

vector<Point2i> FindConeContour(const Mat& sourceImage, const Parameters parameters) {

	vector<vector<Point2i>> contours;
	vector<Vec4i> hierarchy;

	findContours(sourceImage, contours, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE);

	const vector<vector<Point2i>> filteredContours = FilteredContours(contours, parameters.MinContourArea, parameters.MaxContourArea);

	if (filteredContours.empty()) {
		return vector<Point2i>();
	}

	return *BiggestContour(filteredContours);
}

bool ComputeConeDetails(const vector<Point2i>& coneContour, const Parameters& parameters, ConeDetails* output) {

	if (coneContour.empty()) {
		return false;
	}

	const Point2i centroid = ContourCentroid(coneContour);
	const Point2i farthestPoint = FarthestPoint(coneContour, centroid);

	const Point2d centroidPosition = CalculateObjectDisplacement(centroid, parameters.CameraResolution,
		parameters.CameraFov, parameters.CameraOffset, parameters.CameraAngle);

	const Point2d tipPosition = CalculateObjectDisplacement(farthestPoint, parameters.CameraResolution,
		parameters.CameraFov, parameters.CameraOffset, parameters.CameraAngle);

	const double coneAngle = CalculateConeAngle(centroidPosition, tipPosition);

	*output = ConeDetails(centroidPosition, tipPosition, centroid, farthestPoint, coneAngle);
	return true;
}

int main() {

	fmt::print("Starting cone detection.\n");

	VideoCapture videoCapture(0);
	if (!videoCapture.isOpened()) {
		fmt::print("The video capture was not opened.\n");
		return 0;
	}

	Mat image, preProcessedImage;

	Parameters parameters = Parameters();
	CeilingToOdd(parameters.BlurKernelSize);
	CeilingToOdd(parameters.ContourDilation);
	CeilingToOdd(parameters.ContourErosion);
	
	nt::DoublePublisher dblPubFound, dblPubAngle, dblPubX, dblPubY;
	auto inst = nt::NetworkTableInstance::GetDefault();
	auto table = inst.GetTable("rpi");
	//auto pubOp = new PubSubOption();
	inst.StartClient4("example client");
	inst.SetServerTeam(4678);  // where TEAM=190, 294, etc, or use inst.setServer("hostname") or similar
	inst.StartDSClient();  // recommended if running on DS computer; this gets the robot IP from the DS
	dblPubFound = table->GetDoubleTopic("cone_found").Publish();
	dblPubAngle = table->GetDoubleTopic("cone_angle").Publish();
	dblPubX = table->GetDoubleTopic("cone_x").Publish();
	dblPubY = table->GetDoubleTopic("cone_y").Publish();
	int cnt = 0;

	while (true) {

#ifdef PRINT_DATA
		time_point<system_clock> startTime = high_resolution_clock::now();
#endif

		videoCapture.read(image);

#ifdef PRINT_DATA
		time_point<system_clock> readingTime = high_resolution_clock::now();
#endif

		ConeDetails coneDetails{};
		PreProcessImage(image, preProcessedImage, parameters);
		vector<Point2i> coneContour = FindConeContour(preProcessedImage, parameters);
		bool coneFound = ComputeConeDetails(coneContour, parameters, &coneDetails);

		dblPubFound.Set(coneFound);
		dblPubAngle.Set(coneDetails.GetAngle() * 180l / PI);
		dblPubX.Set(coneDetails.GetCentroidPosition().x);
		dblPubY.Set(coneDetails.GetCentroidPosition().y);

#ifdef PRINT_DATA
		time_point<system_clock> endTime = high_resolution_clock::now();
		duration<double, milli> readingDuration = readingTime - startTime;
		duration<double, milli> findingDuration = endTime - readingTime;

		fmt::print(
			"reading: " + FixLength(to_string(readingDuration.count()), 8) +
			",   finding: " + FixLength(to_string(findingDuration.count()), 8) +
			",   x: " + FixLength(to_string(coneDetails.GetCentroidPosition().x), 8) +
			",   y: " + FixLength(to_string(coneDetails.GetCentroidPosition().y), 8) +
			",   angle: " + FixLength(to_string(coneDetails.GetAngle() * 180l / PI), 8) + "\n");
#endif

		waitKey(1);
	}
}
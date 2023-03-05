#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#include <chrono>
#include <iostream>
#include <string>

#include "Colors.h"
#include "ConeDetails.h"
#include "Contours.h"
#include "Parameters.h"
#include "Wrappers.h"
#include "MultiImageWindow.h"
#include "Trigonometry.h"
#include "Points.h"

#define SHOW_UI true;
//#define PRINT_TIME true;

using namespace cv;
using namespace std;
using namespace std::chrono;



void CeilingToOdd(int& number) {

	if (number % 2 == 0) {
		number++;
	}
}

void PreProcessImage(const Mat& sourceImage, Mat& targetImage, Parameters parameters, MultiImageWindow& guiWindow) {

	Mat imageHsv, mask, maskDilated, maskEroded, blurred, edges, contoursDilated, contoursEroded;

	CeilingToOdd(parameters.BlurKernelSize);
	CeilingToOdd(parameters.ContourDilation);
	CeilingToOdd(parameters.ContourErosion);

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

	copyMakeBorder(contoursEroded, targetImage, 1, 1, 1, 1, BORDER_CONSTANT, WHITE);

#ifdef SHOW_UI
	guiWindow.AddImage(mask, 0, 0, "Mask");
	guiWindow.AddImage(maskEroded, 1, 0, "Mask Eroded");
	guiWindow.AddImage(maskDilated, 2, 0, "Mask Dilated");
	guiWindow.AddImage(blurred, 3, 0, "Blur");
	guiWindow.AddImage(edges, 0, 1, "Canny");
	guiWindow.AddImage(contoursDilated, 1, 1, "Dilated");
	guiWindow.AddImage(targetImage, 2, 1, "Eroded, Bordered");
#endif
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

vector<vector<Point2i>> GetConeCornerGroups(const vector<Point2i>& coneContour, const ConeDetails coneDetails) {

	const double distanceToTip = DistanceBetweenPoints(coneDetails.GetCentroidCameraPosition(), coneDetails.GetTipCameraPosition());

	bool startNewCornerGroupOnNextPoint = true;
	vector<vector<Point2i>> cornerGroups = vector<vector<Point2i>>();

	for (Point2i point : coneContour) {

		if (DistanceBetweenPoints(point, coneDetails.GetCentroidCameraPosition()) < distanceToTip * 0.85) {
			startNewCornerGroupOnNextPoint = true;
			continue;
		}

		if (startNewCornerGroupOnNextPoint || cornerGroups.empty()) {
			cornerGroups.emplace_back();
			startNewCornerGroupOnNextPoint = false;
		}

		cornerGroups.back().push_back(point);
	}

	if (cornerGroups.size() < 2) {
		return cornerGroups;
	}

	if (cornerGroups.back().back() == coneContour.back() && cornerGroups.front().front() == coneContour.front()) {

		for (Point2i point : cornerGroups.back()) {
			cornerGroups.front().push_back(point);
		}

		cornerGroups.pop_back();
	}

	return cornerGroups;
}

void DrawConeDetails(Mat& targetImage, const vector<Point2i>& coneContour, const ConeDetails& coneDetails, MultiImageWindow& guiWindow) {

	if (coneContour.empty()) {
		guiWindow.AddImage(targetImage, 3, 1, "Contours");
		return;
	}

	DrawContour(targetImage, coneContour, MAGENTA);

	line(targetImage, coneDetails.GetTipCameraPosition(), coneDetails.GetCentroidCameraPosition(), RED);

	const string text = "X:" + to_string(coneDetails.GetCentroidPosition().x) +
						", Y:" + to_string(coneDetails.GetCentroidPosition().y) +
						", A:" + to_string(coneDetails.GetAngle() * 180 / PI);

	putText(targetImage, text, Point2i(20, 45), 0, 0.75, GREEN);

	guiWindow.AddImage(targetImage, 3, 1, "Contours");
}

int main() {

	vector<VideoCapture> videoCaptures = vector<VideoCapture>();
	for (int i = 0; i < 4; i++) {
		videoCaptures.emplace_back(i);
	}

	Mat image, preProcessedImage;
	MultiImageWindow multiImageWindow = MultiImageWindow("Pipeline", 4, 2);

	Parameters parameters = Parameters();
#ifdef SHOW_UI
	parameters.CreateTrackbars();
#endif

	while (true) {

		videoCaptures[parameters.CameraId].read(image);

		if (image.rows == 0) {
			image = Mat(parameters.CameraResolution.y, parameters.CameraResolution.x, CV_8UC3, RED);
		}

#ifdef PRINT_TIME
		time_point<steady_clock> startTime = high_resolution_clock::now();
#endif

		ConeDetails coneDetails{};
		PreProcessImage(image, preProcessedImage, parameters, multiImageWindow);
		vector<Point2i> coneContour = FindConeContour(preProcessedImage, parameters);
		ComputeConeDetails(coneContour, parameters, &coneDetails);

		vector<vector<Point2i>> cornerGroups = GetConeCornerGroups(coneContour, coneDetails);

#ifdef SHOW_UI
		circle(image, coneDetails.GetCentroidCameraPosition(),
			DistanceBetweenPoints(coneDetails.GetCentroidCameraPosition(), coneDetails.GetTipCameraPosition()) * 0.8, GREEN, 2);

		int colorIndex = 0;
		for (const vector<Point2i>& cornerGroup : cornerGroups) {
			for (Point2i point : cornerGroup) {
				drawMarker(image, point, GetColorByIndex(colorIndex));
			}
			colorIndex++;
		}
#endif

#ifdef SHOW_UI
		DrawConeDetails(image, coneContour, coneDetails, multiImageWindow);
		multiImageWindow.Show(parameters.WindowWidth, parameters.WindowHeight);
#endif

		if (cornerGroups.size() > 3) {
			coneDetails = ConeDetails();
		}

#ifdef PRINT_TIME
		time_point<steady_clock> endTime = high_resolution_clock::now();
		duration<double, milli> duration = endTime - startTime;
		cout << duration.count() << endl;
#endif

		waitKey(1);
	}
}
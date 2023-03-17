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
//#define FROM_WEBCAM true
#define FROM_FILE "cone2.mp4"
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

	Mat imageHsv, masksMerged, masksMergedDigitized, edges, contoursDilated, contoursEroded;
	Mat wideMask, wideMaskEroded, wideMaskDilated, wideMaskBlurred;
	Mat narrowMask, narrowMaskDilated, narrowMaskEroded, narrowMaskBlurred;

	CeilingToOdd(parameters.NarrowBlurKernelSize);
	CeilingToOdd(parameters.WideBlurKernelSize);
	CeilingToOdd(parameters.ContourDilation);
	CeilingToOdd(parameters.ContourErosion);

	Scalar wideLowerColorLimit = Scalar(parameters.WideHueMin, parameters.WideSaturationMin, parameters.WideValueMin);
	Scalar wideUpperColorLimit = Scalar(parameters.WideHueMax, parameters.WideSaturationMax, parameters.WideValueMax);

	Scalar narrowLowerColorLimit = Scalar(parameters.NarrowHueMin, parameters.NarrowSaturationMin, parameters.NarrowValueMin);
	Scalar narrowUpperColorLimit = Scalar(parameters.NarrowHueMax, parameters.NarrowSaturationMax, parameters.NarrowValueMax);

	cvtColor(sourceImage, imageHsv, COLOR_BGR2HSV);
	inRange(imageHsv, wideLowerColorLimit, wideUpperColorLimit, wideMask);
	inRange(imageHsv, narrowLowerColorLimit, narrowUpperColorLimit, narrowMask);

	SquareErode(wideMask, wideMaskEroded, parameters.WideMaskErosion);
	SquareDilate(wideMaskEroded, wideMaskDilated, parameters.WideMaskDilation);

	SquareDilate(narrowMask, narrowMaskDilated, parameters.NarrowMaskDilation);
	SquareErode(narrowMaskDilated, narrowMaskEroded, parameters.NarrowMaskErosion);

	GaussianBlur(wideMaskDilated, wideMaskBlurred, Size(parameters.WideBlurKernelSize, parameters.WideBlurKernelSize), parameters.WideBlurSigmaX, parameters.WideBlurSigmaY);
	GaussianBlur(narrowMaskEroded, narrowMaskBlurred, Size(parameters.NarrowBlurKernelSize, parameters.NarrowBlurKernelSize), parameters.NarrowBlurSigmaX, parameters.NarrowBlurSigmaY);

	masksMerged = parameters.WideMaskWeight / 100.0 * wideMaskBlurred + (100 - parameters.WideMaskWeight) / 100.0 * narrowMaskBlurred;

	inRange(masksMerged, parameters.MaskThreshold, Scalar(255), masksMergedDigitized);

	Canny(masksMergedDigitized, edges, parameters.CannyThreshold1, parameters.CannyThreshold2);

	SquareDilate(edges, contoursDilated, parameters.ContourDilation);
	SquareErode(contoursDilated, contoursEroded, parameters.ContourErosion);

	copyMakeBorder(contoursEroded, targetImage, 1, 1, 1, 1, BORDER_CONSTANT, WHITE);

#ifdef SHOW_UI
	guiWindow.AddImage(wideMask, 0, 0, "W Mask");
	guiWindow.AddImage(wideMaskEroded, 1, 0, "W Mask Eroded");
	guiWindow.AddImage(wideMaskDilated, 2, 0, "W Mask Dilated");
	guiWindow.AddImage(wideMaskBlurred, 3, 0, "W Mask Blurred");

	guiWindow.AddImage(narrowMask, 0, 1, "N Mask");
	guiWindow.AddImage(narrowMaskDilated, 1, 1, "N Mask Dilated");
	guiWindow.AddImage(narrowMaskEroded, 2, 1, "N Mask Eroded");
	guiWindow.AddImage(narrowMaskBlurred, 3, 1, "N Mask Blurred");

	guiWindow.AddImage(masksMerged, 0, 2, "Masks Merged");
	guiWindow.AddImage(masksMergedDigitized, 1, 2, "Digitized Mask");
	//guiWindow.AddImage(edges, 0, 1, "Canny");
	//guiWindow.AddImage(contoursDilated, 1, 1, "Dilated");
	guiWindow.AddImage(targetImage, 2, 2, "Eroded");
#endif
}

vector<Point2i> FindConeContour(const Mat& sourceImage, const Parameters parameters) {

	vector<vector<Point2i>> contours;
	vector<Vec4i> hierarchy;

	findContours(sourceImage, contours, hierarchy, RETR_LIST, CHAIN_APPROX_NONE);

	const vector<vector<Point2i>> filteredContours = FilteredContours(contours, parameters.MinContourArea, parameters.MaxContourArea);

	if (filteredContours.empty()) {
		return vector<Point2i>();
	}

	return *MostCentralAndSmallestContour(filteredContours, parameters.CameraResolution);
}

void GetConeCornerGroups(const vector<Point2i>& coneContour, const Point2i centroidCameraPosition, 
	const Point2i farthestPointCameraPosition, vector<vector<Point2i>>& cornerGroups) {

	const double distanceToTip = DistanceBetweenPoints(centroidCameraPosition, farthestPointCameraPosition);

	cornerGroups = vector<vector<Point2i>>();
	cornerGroups.emplace_back();

	for (int i = 0; i < coneContour.size(); i++) {

		Point2i point = coneContour[i];

		if (DistanceBetweenPoints(point, centroidCameraPosition) < distanceToTip * 0.85) {
			continue;
		}

		if (cornerGroups.back().empty()) {
			cornerGroups.back().push_back(point);
			continue;
		}

		const bool continuationOfPreviousCorner = coneContour[i - 1] == cornerGroups.back().back();
		const bool notTooFarFromPreviousPoint = DistanceBetweenPoints(coneContour[i], cornerGroups.back().back()) < 15;
		const bool closeToPreviousPoint = DistanceBetweenPoints(coneContour[i], cornerGroups.back().back()) < 6;

		if ((continuationOfPreviousCorner && notTooFarFromPreviousPoint) || closeToPreviousPoint) {
			cornerGroups.back().push_back(point);
			continue;
		}

		cornerGroups.emplace_back();
		cornerGroups.back().push_back(point);
	}

	if (cornerGroups.size() < 2) {
		return;
	}

	if (cornerGroups.back().back() == coneContour.back() && cornerGroups.front().front() == coneContour.front()) {

		for (Point2i point : cornerGroups.back()) {
			cornerGroups.front().push_back(point);
		}

		cornerGroups.pop_back();
	}
}

vector<Point2i> GetHighestInEachCornerGroup(const vector<vector<Point2i>>& cornerGroups, Point2i& highest, Point2i& secondHighest) {

	vector<Point2i> northMostInEachCornerGroup{};

	for (const vector<Point2i>& cornerGroup : cornerGroups) {

		northMostInEachCornerGroup.push_back(cornerGroup[0]);

		for (Point2i point : cornerGroup) {

			if (point.y < northMostInEachCornerGroup.back().y) {
				northMostInEachCornerGroup.pop_back();
				northMostInEachCornerGroup.push_back(point);
			}
		}
	}

	highest = northMostInEachCornerGroup[0].y < northMostInEachCornerGroup[1].y ? northMostInEachCornerGroup[0] : northMostInEachCornerGroup[1];
	secondHighest = northMostInEachCornerGroup[0].y > northMostInEachCornerGroup[1].y ? northMostInEachCornerGroup[0] : northMostInEachCornerGroup[1];
	for (int i = 2; i < northMostInEachCornerGroup.size(); i++) {

		if (northMostInEachCornerGroup[i].y < secondHighest.y) {
			secondHighest = northMostInEachCornerGroup[i];
		}

		if (northMostInEachCornerGroup[i].y < highest.y) {
			const Point2i temp = secondHighest;
			secondHighest = highest;
			highest = temp;
		}
	}

	return northMostInEachCornerGroup;
}

Point2i AdjustTipFromCornerPoints(const vector<vector<Point2i>>& cornerGroups, const Point2i currentTipPosition) {

	if (cornerGroups.size() < 4) {
		return currentTipPosition;
	}

	if (cornerGroups.size() == 4) {

		Point2i highestPoint;
		Point2i secondHighestPoint;
		GetHighestInEachCornerGroup(cornerGroups, highestPoint, secondHighestPoint);

		if (highestPoint.y - secondHighestPoint.y < -5) {
			return highestPoint;
		}

		return Point2i((highestPoint.x + secondHighestPoint.x) / 2, (highestPoint.y + secondHighestPoint.y) / 2);
	}

	if (cornerGroups.size() == 5) {

		vector<Point2i> corners{};

		corners.reserve(cornerGroups.size());
		for (const vector<Point2i>& cornerGroup : cornerGroups) {
			corners.push_back(AveragePointInGroup(cornerGroup));
		}

		Point2i highestPoint = corners[0];

		for (const Point2i point : corners) {
			if (point.y < highestPoint.y) {
				highestPoint = point;
			}
		}

		return highestPoint;
	}

	Point2i highestPoint;
	Point2i secondNorthMost;
	GetHighestInEachCornerGroup(cornerGroups, highestPoint, secondNorthMost);

	if (highestPoint.y - secondNorthMost.y < -5) {
		return highestPoint;
	}

	double sumX = 0;
	int numberOfPoints = 0;

	for (const vector<Point2i>& cornerGroup : cornerGroups) {

		for (const Point2i point : cornerGroup) {
			numberOfPoints++;
			sumX += point.x;
		}
	}

	return Point2i(sumX / numberOfPoints, highestPoint.y);
}

bool ComputeConeDetails(const vector<Point2i>& coneContour, const Parameters& parameters, ConeDetails* output, vector<vector<Point2i>>& cornerGroups) {

	if (coneContour.empty()) {
		return false;
	}

	const Point2i centroid = ContourCentroid(coneContour);
	Point2i farthestPoint = FarthestPoint(coneContour, centroid);

	GetConeCornerGroups(coneContour, centroid, farthestPoint, cornerGroups);
	farthestPoint = AdjustTipFromCornerPoints(cornerGroups, farthestPoint);

	const Point2d centroidPosition = CalculateObjectDisplacement(centroid, parameters.CameraResolution, 
		parameters.CameraFov, parameters.CameraOffset, parameters.CameraAngle);

	const Point2d tipPosition = CalculateObjectDisplacement(farthestPoint, parameters.CameraResolution, 
		parameters.CameraFov, parameters.CameraOffset, parameters.CameraAngle);

	const double coneAngle = CalculateConeAngle(centroidPosition, tipPosition);

	const double adjustedConeAngle = ApplyConeTippedErrorCorrection(coneAngle, centroid, parameters.CameraAngle, parameters.CameraResolution, parameters.CameraFov);

	*output = ConeDetails(centroidPosition, tipPosition, centroid, farthestPoint, adjustedConeAngle);
	return true;
}

void DrawConeDetails(Mat& targetImage, const vector<Point2i>& coneContour, const ConeDetails& coneDetails, MultiImageWindow& guiWindow) {

	if (coneContour.empty()) {
		guiWindow.AddImage(targetImage, 3, 2, "Contours");
		return;
	}

	DrawContour(targetImage, coneContour, MAGENTA);

	line(targetImage, coneDetails.GetTipCameraPosition(), coneDetails.GetCentroidCameraPosition(), RED);

	const string text = "X:" + to_string(coneDetails.GetCentroidPosition().x) +
						", Y:" + to_string(coneDetails.GetCentroidPosition().y) +
						", A:" + to_string(coneDetails.GetAngle() * 180 / PI);

	putText(targetImage, text, Point2i(20, 45), 0, 0.75, GREEN);

	guiWindow.AddImage(targetImage, 3, 2, "Contours");
}

int main() {

#ifdef FROM_WEBCAM
	vector<VideoCapture> videoCaptures = vector<VideoCapture>();
	for (int i = 0; i < 4; i++) {
		videoCaptures.emplace_back(i);
	}
#endif

	Mat image, preProcessedImage, blackedOutImage;
	MultiImageWindow multiImageWindow = MultiImageWindow("Pipeline", 4, 3);
	Parameters parameters = Parameters();

#ifdef SHOW_UI
	parameters.CreateTrackbars();
#endif

#ifdef FROM_FILE
	Mat frame;
	vector<Mat> frames;
	VideoCapture videoCapture(FROM_FILE);
	videoCapture.read(frame);

	int currentFrameIndex = 0;
	bool play = true;

	while (frame.rows != 0) {
		frames.push_back(frame.clone());
		videoCapture.read(frame);
	}
#endif

	while (true) {

#ifdef FROM_WEBCAM
		videoCaptures[parameters.CameraId].read(image);

		if (image.rows == 0) {
			image = Mat(parameters.CameraResolution.y, parameters.CameraResolution.x, CV_8UC3, RED);
		}
#endif

#ifdef FROM_FILE
		image = frames[currentFrameIndex].clone();
#endif

#ifdef PRINT_TIME
		time_point<steady_clock> startTime = high_resolution_clock::now();
#endif

		PreProcessImage(image, preProcessedImage, parameters, multiImageWindow);

		ConeDetails coneDetails{};
		vector<vector<Point2i>> cornerGroups{};
		vector<Point2i> coneContour = FindConeContour(preProcessedImage, parameters);
		ComputeConeDetails(coneContour, parameters, &coneDetails, cornerGroups);

#ifdef SHOW_UI
		circle(image, coneDetails.GetCentroidCameraPosition(),
			DistanceBetweenPoints(coneDetails.GetCentroidCameraPosition(), coneDetails.GetTipCameraPosition()) * 0.85, GREEN, 2);

		int colorIndex = 0;
		for (const vector<Point2i>& cornerGroup : cornerGroups) {
			for (Point2i point : cornerGroup) {
				drawMarker(image, point, GetColorByIndex(colorIndex));
			}
			colorIndex++;
		}

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

#ifdef FROM_WEBCAM
		waitKey(1);
#endif


#ifdef FROM_FILE
		int keyPressed = waitKey(1);

		// space
		if (keyPressed == 32) {
			play = !play;
		}

		if (play) {
			currentFrameIndex = min((int)frames.size() - 1, currentFrameIndex + 1);
			continue;
		}

		// >
		if (keyPressed == 46 && currentFrameIndex + 1 < frames.size()) {
			currentFrameIndex = min((int)frames.size() - 1, currentFrameIndex + 1);

		// <
		} else if (keyPressed == 44 && currentFrameIndex > 0) {
			currentFrameIndex = max(0, currentFrameIndex - 1);

		// '
		} else if (keyPressed == 39 && currentFrameIndex + 1 < frames.size()) {
			currentFrameIndex = min((int)frames.size() - 1, currentFrameIndex + 5);

		// ;
		} else if (keyPressed == 59 && currentFrameIndex > 0) {
			currentFrameIndex = max(0, currentFrameIndex - 5);

		// ]
		} else if (keyPressed == 93 && currentFrameIndex + 1 < frames.size()) {
			currentFrameIndex = min((int)frames.size() - 1, currentFrameIndex + 25);

		// [
		} else if (keyPressed == 91 && currentFrameIndex > 0) {
			currentFrameIndex = max(0, currentFrameIndex - 25);
		}
#endif

	}
}
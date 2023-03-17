#include <chrono>
#include <string>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#include <fmt/format.h>
#include <networktables/NetworkTableInstance.h>
#include <networktables/NetworkTable.h>
#include <networktables/DoubleTopic.h>

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
using namespace std::this_thread;



nt::DoublePublisher dblPubFound, dblPubAngle, dblPubX, dblPubY;
nt::NetworkTableInstance inst;

#ifdef PRINT_DATA
string FixLength(const string& text, const char length) {

	const int textLength = text.length();

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

	copyMakeBorder(contoursEroded, targetImage, 1, 1, 1, 1, BORDER_CONSTANT);
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

vector<Point2i> GetHighestInEachCornerGroup(const vector<vector<Point2i>>& cornerGroups, 
	Point2i& highestPointInHighestCorner, Point2i& highestPointInSecondHighestCorner) {

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

	highestPointInHighestCorner = northMostInEachCornerGroup[0].y < northMostInEachCornerGroup[1].y ? northMostInEachCornerGroup[0] : northMostInEachCornerGroup[1];
	highestPointInSecondHighestCorner = northMostInEachCornerGroup[0].y > northMostInEachCornerGroup[1].y ? northMostInEachCornerGroup[0] : northMostInEachCornerGroup[1];
	for (int i = 2; i < northMostInEachCornerGroup.size(); i++) {

		if (northMostInEachCornerGroup[i].y < highestPointInSecondHighestCorner.y) {
			highestPointInSecondHighestCorner = northMostInEachCornerGroup[i];
		}

		if (northMostInEachCornerGroup[i].y < highestPointInHighestCorner.y) {
			const Point2i temp = highestPointInSecondHighestCorner;
			highestPointInSecondHighestCorner = highestPointInHighestCorner;
			highestPointInHighestCorner = temp;
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

void ConnectToNetworkTables() {

#ifdef PRINT_DATA
	fmt::print("Connecting to network tables.\n");
#endif

	inst = nt::NetworkTableInstance::GetDefault();
	auto table = inst.GetTable("rpi");
	//auto pubOp = new PubSubOption();
	inst.StartClient4("example client");
	inst.SetServerTeam(4678);  // where TEAM=190, 294, etc, or use inst.setServer("hostname") or similar
	inst.StartDSClient();  // recommended if running on DS computer; this gets the robot IP from the DS
	dblPubFound = table->GetDoubleTopic("cone_found").Publish();
	dblPubAngle = table->GetDoubleTopic("cone_angle").Publish();
	dblPubX = table->GetDoubleTopic("cone_x").Publish();
	dblPubY = table->GetDoubleTopic("cone_y").Publish();
}

int main() {

	fmt::print("Starting cone detection.\n");
	fmt::print("Waiting for 15 seconds for the rio to boot.\n");

	sleep_for(seconds(15));

	VideoCapture videoCapture(0);
	if (!videoCapture.isOpened()) {
		fmt::print("The video capture was not opened.\n");
		return 0;
	}

	Mat image, preProcessedImage;

	Parameters parameters = Parameters();
	CeilingToOdd(parameters.WideBlurKernelSize);
	CeilingToOdd(parameters.NarrowBlurKernelSize);
	CeilingToOdd(parameters.ContourDilation);
	CeilingToOdd(parameters.ContourErosion);

	ConnectToNetworkTables();

	int iterationCount = 0;
	while (true) {

		iterationCount++;

		if (iterationCount % 50 == 0) {
			ConnectToNetworkTables();
		}

#ifdef PRINT_DATA
		time_point<system_clock> startTime = high_resolution_clock::now();
#endif

		videoCapture.read(image);

#ifdef PRINT_DATA
		time_point<system_clock> readingTime = high_resolution_clock::now();
#endif

		PreProcessImage(image, preProcessedImage, parameters);

		ConeDetails coneDetails{};
		vector<vector<Point2i>> cornerGroups{};
		vector<Point2i> coneContour = FindConeContour(preProcessedImage, parameters);
		bool coneFound = ComputeConeDetails(coneContour, parameters, &coneDetails, cornerGroups);

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
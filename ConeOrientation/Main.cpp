#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <cmath>

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

Point FindFarthestPoint(const vector<Point>& points, const Point basePoint) {

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



double LineAngleFromVertical(const Point center, const Point endpoint) {

	const int deltaX = center.x - endpoint.x;
	const int deltaY = center.y - endpoint.y;

	return atan2(deltaY, deltaX);
}



vector<vector<Point>> FilteredContours(const vector<vector<Point>>& contours, const int minArea, const int maxArea) {

	vector<vector<Point>> filteredContours;

	for (const vector<Point>& contour : contours) {

		const double area = contourArea(contour);

		if (area < minArea && area > maxArea) {
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



void DrawContour(Mat& image, const vector<Point>& contour, const Scalar& color = Scalar(0, 0, 0), const int thickness = 1) {

	vector<vector<Point>> dummy = vector<vector<Point>>();

	dummy.push_back(contour);

	drawContours(image, dummy, 0, color, thickness);
}




void FindFilterDrawContours(const Mat& sourceImage, Mat& targetImage, const int minContourArea, const int maxContourArea) {

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;

	findContours(sourceImage, contours, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE);

	const vector<vector<Point>> filteredContours = FilteredContours(contours, minContourArea, maxContourArea);

	for (int i = 0; i < filteredContours.size(); i++) {
		//drawContours(targetImage, contours, i, Scalar(255, 0, 255));
		//DrawContour(targetImage, contours[i], Scalar(255, 0, 255));
	}

	const vector<Point>* smallestContour = SmallestContour(filteredContours);

	if (smallestContour == nullptr) {
		return;
	}

	DrawContour(targetImage, *smallestContour, Scalar(255, 0, 255));

	const Point centroid = ContourCentroid(*smallestContour);
	const Point farthestPoint = FindFarthestPoint(*smallestContour, centroid);

	const double angle = LineAngleFromVertical(centroid, farthestPoint);

	putText(targetImage, to_string(angle), centroid, 0, 1.0, Scalar(0, 0, 0));

	line(targetImage, farthestPoint, centroid, Scalar(0, 0, 255));
}



int main() {

	constexpr int cameraId = 2;
	VideoCapture videoCapture(cameraId);
	Mat image, mask, imageHsv, imageGray, imageBlur, imageCanny, imageDilated, imageEroded, imageContours;

	int hueMin = 10;
	int hueMax = 25;
	int saturationMin = 150;
	int saturationMax = 255;
	int valueMin = 75;
	int valueMax = 255;

	int blurKernelSize = 3;
	int blurSigmaX = 5;
	int blurSigmaY = 0;

	int cannyThreshold1 = 50;
	int cannyThreshold2 = 150;

	int dilationKernelSize = 9;
	int erosionKernelSize = 1;

	int minContourArea = 10000;
	int maxContourArea = 75000;

	Scalar lowerColorLimit, upperColorLimit;
	Mat dilationKernel, erosionKernel;

	namedWindow("Sliders", 100);

	createTrackbar("Hue Min", "Sliders", &hueMin, 179);
	createTrackbar("Hue Max", "Sliders", &hueMax, 179);
	createTrackbar("Sat Min", "Sliders", &saturationMin, 255);
	createTrackbar("Sat Max", "Sliders", &saturationMax, 255);
	createTrackbar("Value Min", "Sliders", &valueMin, 255);
	createTrackbar("Value Max", "Sliders", &valueMax, 255);

	createTrackbar("Blur", "Sliders", &blurKernelSize, 30);
	createTrackbar("Blur SX", "Sliders", &blurSigmaX, 30);
	createTrackbar("Blur SY", "Sliders", &blurSigmaY, 30);

	createTrackbar("Canny T1", "Sliders", &cannyThreshold1, 250);
	createTrackbar("Canny T2", "Sliders", &cannyThreshold2, 250);

	createTrackbar("Dilation", "Sliders", &dilationKernelSize, 50);
	createTrackbar("Erosion", "Sliders", &erosionKernelSize, 50);

	createTrackbar("Min Area", "Sliders", &minContourArea, 100000);
	createTrackbar("Max Area", "Sliders", &maxContourArea, 100000);

	while (true) {

		if (blurKernelSize % 2 == 0) {
			blurKernelSize++;
		}

		if (dilationKernelSize % 2 == 0) {
			dilationKernelSize++;
		}

		if (erosionKernelSize % 2 == 0) {
			erosionKernelSize++;
		}

		videoCapture.read(image);

		lowerColorLimit = Scalar(hueMin, saturationMin, valueMin);
		upperColorLimit = Scalar(hueMax, saturationMax, valueMax);
		cvtColor(image, imageHsv, COLOR_BGR2HSV);
		inRange(imageHsv, lowerColorLimit, upperColorLimit, mask);

		GaussianBlur(mask, imageBlur, Size(blurKernelSize, blurKernelSize), blurSigmaX, blurSigmaY);
		Canny(imageBlur, imageCanny, cannyThreshold1, cannyThreshold2);

		dilationKernel = getStructuringElement(MORPH_RECT, Size(dilationKernelSize, dilationKernelSize));
		dilate(imageCanny, imageDilated, dilationKernel);

		erosionKernel = getStructuringElement(MORPH_RECT, Size(erosionKernelSize, erosionKernelSize));
		erode(imageDilated, imageEroded, erosionKernel);

		imageContours = image;
		FindFilterDrawContours(imageEroded, imageContours, minContourArea, maxContourArea);

		//imshow("Normal", image);
		imshow("Mask", mask);
		//imshow("Greyscale", imageGray);
		//imshow("Blur", imageBlur);
		imshow("Canny", imageCanny);
		imshow("Dilated", imageDilated);
		imshow("Eroded", imageEroded);
		imshow("Contours", imageContours);

		waitKey(1);
	}
}
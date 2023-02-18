#pragma once

#include <opencv2/highgui.hpp>
#include "Trigonometry.h"

using namespace cv;



class Parameters {

	public:

		int HueMin = 10;
		int HueMax = 30;
		int SaturationMin = 55;
		int SaturationMax = 255;
		int ValueMin = 171;
		int ValueMax = 255;

		int MaskErosion = 11;
		int MaskDilation = 11;

		int BlurKernelSize = 6;
		int BlurSigmaX = 5;
		int BlurSigmaY = 0;
		 
		int CannyThreshold1 = 50;
		int CannyThreshold2 = 150;
		 
		int ContourDilation = 5;
		int ContourErosion = 3;
		 
		int MinContourArea = 2000;
		int MaxContourArea = 75000;

		const Point2i CameraResolution = Point2i(640, 480) + Point2i(2, 2); // plus 2 to each for the borders
		const Point2d CameraFov = Point2d(54.18l / 180.0l * PI, 39.93l / 180.0l * PI); // 3.6mm ELP
		//const Point2d CameraFov = Point2d(48.5l / 180.0l * PI, 36.0l / 180.0l * PI); // Microsoft Lifecam HD 3000
		
		Point3d CameraOffset = Point3d(0, -3, 52);
		Point2d CameraAngle = Point2d(0.0l / 180.0l * PI, -60.0l / 180.01 * PI);

};
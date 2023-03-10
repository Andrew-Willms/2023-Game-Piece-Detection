#pragma once

#include "Trigonometry.h"

using namespace cv;



class Parameters {

	public:
									
		int WideHueMin = 10;
		int WideHueMax = 35;
		int WideSaturationMin = 10;
		int WideSaturationMax = 255;
		int WideValueMin = 200;
		int WideValueMax = 255;
		int WideMaskErosion = 3;
		int WideMaskDilation = 3;
		int WideBlurKernelSize = 30;
		int WideBlurSigmaX = 5;
		int WideBlurSigmaY = 0;
		int WideMaskWeight = 40;

		int NarrowHueMin = 20;
		int NarrowHueMax = 35;
		int NarrowSaturationMin = 50;
		int NarrowSaturationMax = 255;
		int NarrowValueMin = 210;
		int NarrowValueMax = 255;
		int NarrowMaskErosion = 3;
		int NarrowMaskDilation = 3;
		int NarrowBlurKernelSize = 30;
		int NarrowBlurSigmaX = 5;
		int NarrowBlurSigmaY = 0;

		int MaskThreshold = 112;

		int CannyThreshold1 = 100;
		int CannyThreshold2 = 150;
								
		int ContourDilation = 3;
		int ContourErosion = 0;
								
		int MinContourArea = 2750;
		int MaxContourArea = 9250;

		const Point2i CameraResolution = Point2i(640, 480) + Point2i(2, 2); // plus 2 to each for the borders
		const Point2d CameraFov = Point2d(54.18l / 180.0l * PI, 39.93l / 180.0l * PI); // 3.6mm ELP
		//const Point2d CameraFov = Point2d(48.5l / 180.0l * PI, 36.0l / 180.0l * PI); // Microsoft Lifecam HD 3000
		
		Point3d CameraOffset = Point3d(0, -3, 52);
		Point2d CameraAngle = Point2d(0.0l / 180.0l * PI, -40.0l / 180.01 * PI);

};
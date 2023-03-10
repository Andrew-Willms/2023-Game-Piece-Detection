#pragma once

#include "Trigonometry.h"

using namespace cv;



class Parameters {

	public:
									
		int WideHueMin = 0;
		int WideHueMax = 179;
		int WideSaturationMin = 0;
		int WideSaturationMax = 255;
		int WideValueMin = 160;
		int WideValueMax = 255;
		int WideMaskErosion = 7;
		int WideMaskDilation = 5;
		int WideBlurKernelSize = 30;
		int WideBlurSigmaX = 5;
		int WideBlurSigmaY = 0;
		int WideMaskWeight = 45;

		int NarrowHueMin = 0;
		int NarrowHueMax = 50;
		int NarrowSaturationMin = 0;
		int NarrowSaturationMax = 255;
		int NarrowValueMin = 200;
		int NarrowValueMax = 255;
		int NarrowMaskErosion = 5;
		int NarrowMaskDilation = 5;
		int NarrowBlurKernelSize = 30;
		int NarrowBlurSigmaX = 5;
		int NarrowBlurSigmaY = 0;

		int MaskThreshold = 108;

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
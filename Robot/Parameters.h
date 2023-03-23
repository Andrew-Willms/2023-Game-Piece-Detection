#pragma once

#include "Trigonometry.h"

using namespace cv;



class Parameters {

	public:
									
		int MiddleHueMin = 8;
		int MiddleHueMax = 30;
		int MiddleSaturationMin = 150;
		int MiddleSaturationMax = 255;
		int MiddleValueMin = 130;
		int MiddleValueMax = 255;
		int MiddleMaskWeight = 100;

		int HighlightHueMin = 4;
		int HighlightHueMax = 30;
		int HighlightSaturationMin = 40;
		int HighlightSaturationMax = 255;
		int HighlightValueMin = 240;
		int HighlightValueMax = 255;
		int HighlightMaskWeight = 100;

		int LowLightHueMin = 3;
		int LowLightHueMax = 31;
		int LowLightSaturationMin = 255;
		int LowLightSaturationMax = 255;
		int LowLightValueMin = 104;
		int LowLightValueMax = 255;
		int LowLightMaskWeight = 100;

		int MaskThreshold = 120;
		int TotalMaskBlur = 7;

		int CannyThreshold1 = 100;
		int CannyThreshold2 = 150;

		int ContourDilation = 3;

		int MinContourArea = 2750;
		int MaxContourArea = 9250;

		const Point2i CameraResolution = Point2i(640, 480) + Point2i(2, 2); // plus 2 to each for the borders
		const Point2d CameraFov = Point2d(54.18l / 180.0l * PI, 39.93l / 180.0l * PI); // 3.6mm ELP
		//const Point2d CameraFov = Point2d(48.5l / 180.0l * PI, 36.0l / 180.0l * PI); // Microsoft Lifecam HD 3000

		Point3d CameraOffset = Point3d(0, -6.75, 52);
		Point2d CameraAngle = Point2d(0.0l / 180.0l * PI, -60.0l / 180.01 * PI);

};
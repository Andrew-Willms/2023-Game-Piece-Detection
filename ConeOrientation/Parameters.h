#pragma once

#include <opencv2/highgui.hpp>
#include "Trigonometry.h"

using namespace cv;



class Parameters {

	public:

		int CameraId = 1;
		int WindowWidth = 2039;// 1912;
		int WindowHeight = 1080;// 1009;

		int MiddleHueMin = 25;
		int MiddleHueMax = 35;
		int MiddleSaturationMin = 21;
		int MiddleSaturationMax = 255;
		int MiddleValueMin = 190;
		int MiddleValueMax = 255;
		int MiddleMaskWeight = 100;

		int HighlightHueMin = 0;
		int HighlightHueMax = 170;
		int HighlightSaturationMin = 0;
		int HighlightSaturationMax = 46;
		int HighlightValueMin = 210;
		int HighlightValueMax = 255;
		int HighlightMaskWeight = 100;

		int LowLightHueMin = 4;
		int LowLightHueMax = 35;
		int LowLightSaturationMin = 130;
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

#ifdef SHOW_UI
		void CreateTrackbars() {

			namedWindow("General", WINDOW_NORMAL);
			namedWindow("Wide Mask", WINDOW_NORMAL);
			namedWindow("Highlight Mask", WINDOW_NORMAL);
			namedWindow("Low Light Mask", WINDOW_NORMAL);

			createTrackbar("CameraId", "General", &CameraId, 3);
			createTrackbar("Window W", "General", &WindowWidth, 4000);
			createTrackbar("Window H", "General", &WindowHeight, 2000);
			createTrackbar("Spacer", "General", &WindowHeight, 2000);

			createTrackbar("M H Min", "Wide Mask", &MiddleHueMin, 179);
			createTrackbar("M H Max", "Wide Mask", &MiddleHueMax, 179);
			createTrackbar("M S Min", "Wide Mask", &MiddleSaturationMin, 255);
			createTrackbar("M S Max", "Wide Mask", &MiddleSaturationMax, 255);
			createTrackbar("M V Min", "Wide Mask", &MiddleValueMin, 255);
			createTrackbar("M V Max", "Wide Mask", &MiddleValueMax, 255);
			createTrackbar("M Weight", "Wide Mask", &MiddleMaskWeight, 100);

			createTrackbar("H H Min", "Highlight Mask", &HighlightHueMin, 179);
			createTrackbar("H H Max", "Highlight Mask", &HighlightHueMax, 179);
			createTrackbar("H S Min", "Highlight Mask", &HighlightSaturationMin, 255);
			createTrackbar("H S Max", "Highlight Mask", &HighlightSaturationMax, 255);
			createTrackbar("H V Min", "Highlight Mask", &HighlightValueMin, 255);
			createTrackbar("H V Max", "Highlight Mask", &HighlightValueMax, 255);
			createTrackbar("H Weight", "Highlight Mask", &HighlightMaskWeight, 100);

			createTrackbar("L H Min", "Low Light Mask", &LowLightHueMin, 179);
			createTrackbar("L H Max", "Low Light Mask", &LowLightHueMax, 179);
			createTrackbar("L S Min", "Low Light Mask", &LowLightSaturationMin, 255);
			createTrackbar("L S Max", "Low Light Mask", &LowLightSaturationMax, 255);
			createTrackbar("L V Min", "Low Light Mask", &LowLightValueMin, 255);
			createTrackbar("L V Max", "Low Light Mask", &LowLightValueMax, 255);
			createTrackbar("L Weight", "Low Light Mask", &LowLightMaskWeight, 100);

			createTrackbar("Threshold", "General", &MaskThreshold, 255);

			createTrackbar("C Dilation", "General", &ContourDilation, 50);

			createTrackbar("Min Area", "General", &MinContourArea, 100000);
			createTrackbar("Max Area", "General", &MaxContourArea, 100000);
			createTrackbar("Mask Blur", "General", &TotalMaskBlur, 30);
		}
#endif

};
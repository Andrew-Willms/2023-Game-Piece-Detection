#pragma once

#include <opencv2/highgui.hpp>
#include "Trigonometry.h"

using namespace cv;



class Parameters {

	public:

		int CameraId = 0;
		int WindowWidth = 1912; //2039
		int WindowHeight = 1009; //1080
									
		int WideHueMin = 0;// 10;
		int WideHueMax = 140;// 35;
		int WideSaturationMin = 0;
		int WideSaturationMax = 255;
		int WideValueMin = 200;
		int WideValueMax = 255;
		int WideMaskErosion = 3;
		int WideMaskDilation = 3;
		int WideBlurKernelSize = 14;// 30;
		int WideBlurSigmaX = 5;
		int WideBlurSigmaY = 0;
		int WideMaskWeight = 25;// 50;

		int NarrowHueMin = 10;// 130;
		int NarrowHueMax = 35;// 140;
		int NarrowSaturationMin = 0;
		int NarrowSaturationMax = 255;
		int NarrowValueMin = 210;// 200;
		int NarrowValueMax = 255;
		int NarrowMaskErosion = 3;
		int NarrowMaskDilation = 3;
		int NarrowBlurKernelSize = 14;// 30;
		int NarrowBlurSigmaX = 5;
		int NarrowBlurSigmaY = 0;

		int MaskThreshold = 120;// 90;

		int CannyThreshold1 = 100;
		int CannyThreshold2 = 150;
								
		int ContourDilation = 3;
		int ContourErosion = 0;
								
		int MinContourArea = 2750;
		int MaxContourArea = 9250;

		const Point2i CameraResolution = Point2i(640, 480) + Point2i(2, 2); // plus 2 to each for the borders
		const Point2d CameraFov = Point2d(54.18l / 180.0l * PI, 39.93l / 180.0l * PI); // 3.6mm ELP
		//const Point2d CameraFov = Point2d(48.5l / 180.0l * PI, 36.0l / 180.0l * PI); // Microsoft Lifecam HD 3000
		
		Point3d CameraOffset = Point3d(0, -6.75, 52);
		Point2d CameraAngle = Point2d(0.0l / 180.0l * PI, -60.0l / 180.01 * PI);

		void CreateTrackbars() {

			namedWindow("General", WINDOW_NORMAL);
			namedWindow("Wide Mask", WINDOW_NORMAL);
			namedWindow("Narrow Mask", WINDOW_NORMAL);

			createTrackbar("CameraId", "General", &CameraId, 3);
			createTrackbar("Window W", "General", &WindowWidth, 4000);
			createTrackbar("Window H", "General", &WindowHeight, 2000);
			createTrackbar("Spacer", "General", &WindowHeight, 2000);

			createTrackbar("W H Min", "Wide Mask", &WideHueMin, 179);
			createTrackbar("W H Max", "Wide Mask", &WideHueMax, 179);
			createTrackbar("W S Min", "Wide Mask", &WideSaturationMin, 255);
			createTrackbar("W S Max", "Wide Mask", &WideSaturationMax, 255);
			createTrackbar("W V Min", "Wide Mask", &WideValueMin, 255);
			createTrackbar("W V Max", "Wide Mask", &WideValueMax, 255);
			createTrackbar("W Erosion", "Wide Mask", &WideMaskErosion, 50);
			createTrackbar("W Dilation", "Wide Mask", &WideMaskDilation, 50);
			createTrackbar("W Blur", "Wide Mask", &WideBlurKernelSize, 50);
			createTrackbar("W Blur SX", "Wide Mask", &WideBlurSigmaX, 30);
			createTrackbar("W Blur SY", "Wide Mask", &WideBlurSigmaY, 30);
			createTrackbar("W Weight", "Wide Mask", &WideMaskWeight, 100);

			createTrackbar("N H Min", "Narrow Mask", &NarrowHueMin, 179);
			createTrackbar("N H Max", "Narrow Mask", &NarrowHueMax, 179);
			createTrackbar("N S Min", "Narrow Mask", &NarrowSaturationMin, 255);
			createTrackbar("N S Max", "Narrow Mask", &NarrowSaturationMax, 255);
			createTrackbar("N V Min", "Narrow Mask", &NarrowValueMin, 255);
			createTrackbar("N V Max", "Narrow Mask", &NarrowValueMax, 255);
			createTrackbar("N Dilation", "Narrow Mask", &NarrowMaskDilation, 50);
			createTrackbar("N Erosion", "Narrow Mask", &NarrowMaskErosion, 50);
			createTrackbar("N Blur", "Narrow Mask", &NarrowBlurKernelSize, 50);
			createTrackbar("N Blur SX", "Narrow Mask", &NarrowBlurSigmaX, 30);
			createTrackbar("N Blur SY", "Narrow Mask", &NarrowBlurSigmaY, 30);

			createTrackbar("Threshold", "General", &MaskThreshold, 255);

			createTrackbar("Canny T1", "General", &CannyThreshold1, 250);
			createTrackbar("Canny T2", "General", &CannyThreshold2, 250);

			createTrackbar("C Dilation", "General", &ContourDilation, 50);
			createTrackbar("C Erosion", "General", &ContourErosion, 50);

			createTrackbar("Min Area", "General", &MinContourArea, 100000);
			createTrackbar("Max Area", "General", &MaxContourArea, 100000);
		}

};
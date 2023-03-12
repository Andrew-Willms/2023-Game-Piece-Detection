#pragma once

#include <opencv2/highgui.hpp>
#include "Trigonometry.h"

using namespace cv;



class Parameters {

	public:

		int CameraId = 0;
		int WindowWidth = 1912; //2039
		int WindowHeight = 1009; //1080
									
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
		
		Point3d CameraOffset = Point3d(0, -7, 52);
		Point2d CameraAngle = Point2d(0.0l / 180.0l * PI, -60.0l / 180.01 * PI);

		void CreateTrackbars() {

			namedWindow("Sliders", 100);

			createTrackbar("CameraId", "Sliders", &CameraId, 3);
			createTrackbar("Window W", "Sliders", &WindowWidth, 4000);
			createTrackbar("Window H", "Sliders", &WindowHeight, 2000);
			createTrackbar("Spacer", "Sliders", &WindowHeight, 2000);

			createTrackbar("W H Min", "Sliders", &WideHueMin, 179);
			createTrackbar("W H Max", "Sliders", &WideHueMax, 179);
			createTrackbar("W S Min", "Sliders", &WideSaturationMin, 255);
			createTrackbar("W S Max", "Sliders", &WideSaturationMax, 255);
			createTrackbar("W V Min", "Sliders", &WideValueMin, 255);
			createTrackbar("W V Max", "Sliders", &WideValueMax, 255);
			createTrackbar("W Erosion", "Sliders", &WideMaskErosion, 50);
			createTrackbar("W Dilation", "Sliders", &WideMaskDilation, 50);
			createTrackbar("W Blur", "Sliders", &WideBlurKernelSize, 50);
			createTrackbar("W Blur SX", "Sliders", &WideBlurSigmaX, 30);
			createTrackbar("W Blur SY", "Sliders", &WideBlurSigmaY, 30);
			createTrackbar("W Weight", "Sliders", &WideMaskWeight, 100);

			createTrackbar("N H Min", "Sliders", &NarrowHueMin, 179);
			createTrackbar("N H Max", "Sliders", &NarrowHueMax, 179);
			createTrackbar("N S Min", "Sliders", &NarrowSaturationMin, 255);
			createTrackbar("N S Max", "Sliders", &NarrowSaturationMax, 255);
			createTrackbar("N V Min", "Sliders", &NarrowValueMin, 255);
			createTrackbar("N V Max", "Sliders", &NarrowValueMax, 255);
			createTrackbar("N Dilation", "Sliders", &NarrowMaskDilation, 50);
			createTrackbar("N Erosion", "Sliders", &NarrowMaskErosion, 50);
			createTrackbar("N Blur", "Sliders", &NarrowBlurKernelSize, 50);
			createTrackbar("N Blur SX", "Sliders", &NarrowBlurSigmaX, 30);
			createTrackbar("N Blur SY", "Sliders", &NarrowBlurSigmaY, 30);

			createTrackbar("Threshold", "Sliders", &MaskThreshold, 255);

			createTrackbar("Canny T1", "Sliders", &CannyThreshold1, 250);
			createTrackbar("Canny T2", "Sliders", &CannyThreshold2, 250);

			createTrackbar("C Dilation", "Sliders", &ContourDilation, 50);
			createTrackbar("C Erosion", "Sliders", &ContourErosion, 50);

			createTrackbar("Min Area", "Sliders", &MinContourArea, 100000);
			createTrackbar("Max Area", "Sliders", &MaxContourArea, 100000);
		}

};
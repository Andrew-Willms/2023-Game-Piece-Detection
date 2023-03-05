﻿#pragma once

#include <opencv2/highgui.hpp>
#include "Trigonometry.h"

using namespace cv;



class Parameters {

	public:

		int CameraId = 0;
		int WindowWidth = 1900;
		int WindowHeight = 1000;
											//  3.6 ELP      |	JIGA	|
		int HueMin = 2;						//	10			 |	6		|
		int HueMax = 30;					//	30			 |	30		|
		int SaturationMin = 0;				//	55			 |	0		|
		int SaturationMax = 255;			//	255			 |	140		|
		int ValueMin = 100;					//	171			 |	176		|
		int ValueMax = 255;					//	255			 |	255		|
											//				 |			|
		int MaskErosion = 7;				//	11			 |			|
		int MaskDilation = 7;				//	11			 |			|
											//				 |			|
		int BlurKernelSize = 11;			//	6			 |			|
		int BlurSigmaX = 5;					//	5			 |			|
		int BlurSigmaY = 0;					//	0			 |			|
											//				 |			|
		int CannyThreshold1 = 100;			//	50			 |			|
		int CannyThreshold2 = 150;			//	150			 |			|
											//				 |			|
		int ContourDilation = 7;			//  5			 |			|
		int ContourErosion = 3;				//	3			 |			|
											//				 |			|
		int MinContourArea = 2000;			//	2000		 |			|
		int MaxContourArea = 75000;			//	75000		 |			|

		const Point2i CameraResolution = Point2i(640, 480) + Point2i(2, 2); // plus 2 to each for the borders
		const Point2d CameraFov = Point2d(54.18l / 180.0l * PI, 39.93l / 180.0l * PI); // 3.6mm ELP
		//const Point2d CameraFov = Point2d(48.5l / 180.0l * PI, 36.0l / 180.0l * PI); // Microsoft Lifecam HD 3000
		
		Point3d CameraOffset = Point3d(0, -3, 52);
		Point2d CameraAngle = Point2d(0.0l / 180.0l * PI, -60.0l / 180.01 * PI);

		void CreateTrackbars() {

			namedWindow("Sliders", 100);

			createTrackbar("CameraId", "Sliders", &CameraId, 3);
			createTrackbar("Window Width", "Sliders", &WindowWidth, 4000);
			createTrackbar("Window Height", "Sliders", &WindowHeight, 2000);

			createTrackbar("Hue Min", "Sliders", &HueMin, 179);
			createTrackbar("Hue Max", "Sliders", &HueMax, 179);
			createTrackbar("Sat Min", "Sliders", &SaturationMin, 255);
			createTrackbar("Sat Max", "Sliders", &SaturationMax, 255);
			createTrackbar("Value Min", "Sliders", &ValueMin, 255);
			createTrackbar("Value Max", "Sliders", &ValueMax, 255);

			createTrackbar("M Erosion", "Sliders", &MaskErosion, 50);
			createTrackbar("M Dilation", "Sliders", &MaskDilation, 50);

			createTrackbar("Blur", "Sliders", &BlurKernelSize, 30);
			createTrackbar("Blur SX", "Sliders", &BlurSigmaX, 30);
			createTrackbar("Blur SY", "Sliders", &BlurSigmaY, 30);

			createTrackbar("Canny T1", "Sliders", &CannyThreshold1, 250);
			createTrackbar("Canny T2", "Sliders", &CannyThreshold2, 250);

			createTrackbar("C Dilation", "Sliders", &ContourDilation, 50);
			createTrackbar("C Erosion", "Sliders", &ContourErosion, 50);

			createTrackbar("Min Area", "Sliders", &MinContourArea, 100000);
			createTrackbar("Max Area", "Sliders", &MaxContourArea, 100000);
		}

};
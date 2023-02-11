#pragma once

#include <opencv2/core/types.hpp>

using namespace cv;
using namespace std;



class ConeDetails {

		Point2d CentroidPosition;
		Point2d TipPosition;
		Point2i CentroidCameraPosition;
		Point2i TipCameraPosition;
		double Angle;

	public:

		ConeDetails(const Point2d centroidPosition, const Point2d tipPosition, const Point2i centroidCameraPosition,
			const Point2i tipCameraPosition, const double angle) {

			CentroidPosition = centroidPosition;
			TipPosition = tipPosition;
			CentroidCameraPosition = centroidCameraPosition;
			TipCameraPosition = tipCameraPosition;
			Angle = angle;
		}

		Point2d GetCentroidPosition() const {
			return CentroidPosition;
		}

		Point2i GetCentroidCameraPosition() const {
			return CentroidCameraPosition;
		}

		Point2d GetTipPosition() const {
			return TipPosition;
		}

		Point2i GetTipCameraPosition() const {
			return TipCameraPosition;
		}

		double GetAngle() const {
			return Angle;
		}

};
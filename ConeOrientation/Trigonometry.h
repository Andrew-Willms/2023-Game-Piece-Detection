﻿#pragma once

#define PI 3.14159
#define MAXIMUM_ERROR_CAUSED_BY_CONE_TIP (19.14 * PI / 180)

#include <opencv2/core/types.hpp>

using namespace std;
using namespace cv;



/**
 * \brief Calculates the real world angle between a specified point in an image and the center axis of the camera.
 * \param cameraCoordinate The in camera coordinate of the point to locate. The coordinate (0, 0) would be the top left corner of the image.
 * \param cameraResolution A point with the X and Y members representing the horizontal and vertical resolution of the camera, respectively.
 * \param cameraFov A point with the X and Y members representing the horizontal and vertical FOV of the camera, respectively in radians.
 * \return A point with the X and Y members representing the yaw and pitch, respectively of the specified point from the the center axis of the camera.
 *  Positive yaw is produced by points in the right half of the image. Positive pitch is produced by points in the upper half of the image.
 */
inline Point2d AngleFromCameraCenter(const Point2i cameraCoordinate, const Point2i cameraResolution, const Point2d cameraFov) {

	const Point2i cameraMidpoint = Point2i(cameraResolution / 2);

	const Point2i cameraCoordinateFromCenter = cameraCoordinate - cameraMidpoint;

	const Point2i cameraCoordinatePositiveYUp = Point2i(cameraCoordinateFromCenter.x, -cameraCoordinateFromCenter.y);

	const Point2d cameraCoordinateFraction = Point2d(
		(double)cameraCoordinatePositiveYUp.x / cameraResolution.x,
		(double)cameraCoordinatePositiveYUp.y / cameraResolution.y
	);

	return Point2d(cameraCoordinateFraction.x * cameraFov.x, cameraCoordinateFraction.y * cameraFov.y);
}

/**
 * \brief Calculates the displacement between an object in camera and the center of the robot.
 * \param cameraCoordinate The in camera coordinate of the object. The coordinate (0, 0) would be the top left corner of the image.
 * \param cameraResolution A point with the X and Y members representing the horizontal and vertical resolution of the camera, respectively.
 * \param cameraFov A point with the X and Y members representing the horizontal and vertical FOV of the camera, respectively in radians.
 * \param cameraOffset A point with the X, Y, and Z members representing the horizontal distance from the camera to the robot center, the forwards/
 *  backwards distance from the camera to the robot center, and the vertical distance from the camera to the ground, respectively. X is positive if the
 *  camera is to the right of the center. Y is positive if the camera is in front of the robot center. Z is positive if the camera is above the ground.
 * \param cameraAngle A point with the X and Y members representing the yaw and pitch, respectively in radians of the camera relative to the robot.
 * \return A point with X and Y representing the left/right and horizontal distances, respectively from the specified object to the center of the robot.
 *  X is positive when the object is to the right of the robot center. Y is positive when the object is in front of the robot center.
 */
inline Point2d CalculateObjectDisplacement(
	const Point2i cameraCoordinate,
	const Point2i cameraResolution,
	const Point2d cameraFov,
	const Point3d cameraOffset,
	const Point2d cameraAngle) {

	const Point2d inCameraAngleToObject = AngleFromCameraCenter(cameraCoordinate, cameraResolution, cameraFov);
	const double yawFromRobotToObject = cameraAngle.x + inCameraAngleToObject.x;

	const double yFromCameraToObject = cameraOffset.z / tan(abs(inCameraAngleToObject.y + cameraAngle.y));
	const double hypotenuseOnYzPlaneToObject = sqrt(cameraOffset.z*cameraOffset.z + yFromCameraToObject*yFromCameraToObject);
	const double xFromCameraToObject = hypotenuseOnYzPlaneToObject * tan(yawFromRobotToObject);

	//double distanceFromCameraToObject = cameraOffset.z / tan(abs(inCameraAngleToObject.y + cameraAngle.y));
	//const double yFromCameraToObject = distanceFromCameraToObject * cos(yawFromRobotToObject);
	//const double xFromCameraToObject = distanceFromCameraToObject * sin(yawFromRobotToObject);

	const Point2d displacementFromRobotCenter = Point2d(xFromCameraToObject + cameraOffset.x, yFromCameraToObject + cameraOffset.y);
	return displacementFromRobotCenter;
}

/**
 * \brief Calculates a desired point along the line defined by the centroid and tip of the cone.
 * \param coneCentroid A point with the X and Y members representing the horizontal and forwards/backwards distances, respectively from the centroid of the
 *  cone to the robot center. X is positive if the centroid is to the right of the robot center. Y is positive if the centroid is in front of the robot center.
 * \param coneTip A point with the X and Y members representing the horizontal and forwards/backwards distances, respectively from the tip of the cone to the
 *  center of the robot. X is positive if the tip is to the right of the robot center. Y is positive if the tip is in front of the robot center.
 * \param proximityToBase A value representing how close the output point should be to the centroid of the cone. If the value is 1 the output
 *  point will be at the cone centroid. If the value is 0 the output point will be at the cone tip.
 * \return A point with the X and Y members representing the horizontal and forwards/backwards distances, respectively of the computed point relative to the
 *  robot center. X is positive if the point is to the right of the robot center. Y is positive if the point is in front of the robot center.
 */
inline Point2d CalculateConeMidpoint(const Point2d coneCentroid, const Point2d coneTip, const double proximityToBase = 0.5) {
	return coneCentroid * proximityToBase + coneTip * (1 - proximityToBase);
}

/**
 * \brief Calculates the angle in radians of the cone relative to the robot.
 * \param coneCentroid A point with the X and Y members representing the horizontal and forwards/backwards distances, respectively from the centroid of the
 *  cone to the robot center. X is positive if the centroid is to the right of the robot center. Y is positive if the centroid is in front of the robot center.
 * \param coneTip A point with the X and Y members representing the horizontal and forwards/backwards distances, respectively from the tip of the cone to the
 *  center of the robot. X is positive if the tip is to the right of the robot center. Y is positive if the tip is in front of the robot center.
 * \return A value representing the angle in radians of the cone relative to the robot. If the value is 0 the cone is inline with the robot and the tip of the
 *  cone is pointing away from the robot. If the value is positive the tip of the cone is to the right of the centroid of the cone from the perspective of the robot.
 */
inline double CalculateConeAngle(const Point2d coneCentroid, const Point2d coneTip) {

	double coneTipOffsetX;
	if (coneTip.x - coneCentroid.x == 0.0L) {
		coneTipOffsetX = 0.0000000001L;

	} else {
		coneTipOffsetX = coneTip.x - coneCentroid.x;
	}

	const Point2d coneTipOffset = Point2d(coneTipOffsetX, coneTip.y - coneCentroid.y);

	// X and Y are deliberately in the opposite named parameter
	const double angle = atan2(coneTipOffset.x, coneTipOffset.y);

	return angle;
}

/**
 * \brief Calculates the angle in radians of the cone relative to the robot compensating for the fact that the cone is 3d dimensional and
 * the contour of the cone seen by the camera is not an flat image on the ground.
 * \param coneAngle The angle in radians of the cone relative to the robot not compensating for the 3d nature of the situation.
 * \param centroidCameraCoordinate The in camera coordinate of the centroid of the cone. The coordinate (0, 0) would be the top left corner of the image.
 * \param cameraAngle A point with the X and Y members representing the yaw and pitch, respectively in radians of the camera relative to the robot.
 * \param cameraResolution A point with the X and Y members representing the horizontal and vertical resolution of the camera, respectively.
 * \param cameraFov A point with the X and Y members representing the horizontal and vertical FOV of the camera, respectively in radians.
 * \return The angle in radians of the cone relative to the robot compensating for the fact that the cone is 3d dimensional and
 * the contour of the cone seen by the camera is not an flat image on the ground.
 */
inline double ApplyConeTippedErrorCorrection(
	const double coneAngle, 
	const Point2i centroidCameraCoordinate,
	const Point2i cameraAngle,
	const Point2i cameraResolution, 
	const Point2i cameraFov) {

	const Point2d inCameraAngleToObject = AngleFromCameraCenter(centroidCameraCoordinate, cameraResolution, cameraFov);
	const double pitchFromCamera = inCameraAngleToObject.y + cameraAngle.y;

	const double pitchErrorFactor = cos(pitchFromCamera);

	// coneAngle / -abs(coneAngle) evaluates to 1 with the opposite sign to the existing coneAngle
	const double yawErrorFactorPreliminary = coneAngle / -abs(coneAngle) * sin(abs(coneAngle));

	const double coneAngleErrorPreliminary = MAXIMUM_ERROR_CAUSED_BY_CONE_TIP * pitchErrorFactor * yawErrorFactorPreliminary;

	const double adjustedConeAnglePreliminary = coneAngle + coneAngleErrorPreliminary;

	const double yawErrorFactor = coneAngle / -abs(coneAngle) * sin(abs(adjustedConeAnglePreliminary));

	const double coneAngleError = MAXIMUM_ERROR_CAUSED_BY_CONE_TIP * pitchErrorFactor * yawErrorFactor;

	return coneAngle + coneAngleError;
}
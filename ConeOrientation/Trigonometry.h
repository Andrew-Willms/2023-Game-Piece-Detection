#pragma once

#include <opencv2/core/types.hpp>

using namespace std;
using namespace cv;



/**
 * \brief Computes the real world angle between a specified point in an image and the center axis of the camera.
 * \param cameraCoordinate The in camera coordinate of the point to locate. The coordinate (0, 0) would be the top left corner of the image.
 * \param cameraResolution A point with the X and Y members representing the horizontal and vertical resolution of the camera, respectively.
 * \param cameraFov A point with the X and Y members representing the horizontal and vertical FOV of the camera, respectively.
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
 * \brief Calculate the displacement between an object in camera and the center of the robot.
 * \param cameraCoordinate The in camera coordinate of the object. The coordinate (0, 0) would be the top left corner of the image.
 * \param cameraResolution A point with the X and Y members representing the horizontal and vertical resolution of the camera, respectively.
 * \param cameraFov A point with the X and Y members representing the horizontal and vertical FOV of the camera, respectively.
 * \param cameraOffset A point with the X, Y, and Z members representing the horizontal distance from the camera to the robot center, the forwards/
 *  backwards distance from the camera to the robot center, and the vertical distance from the camera to the ground, respectively. X is positive if the
 *  camera is to the right of the center. Y is positive if the camera is in front of the robot center. Z is positive if the camera is above the ground.
 * \param cameraAngle A point with the X and Y members representing the yaw and pitch, respectively in radians of the camera relative to the robot.
 * \return A point with X and Y representing the left/right and forwards backwards distance, respectively from the specified object to the center of the robot.
 *  X is positive when the object is to the right of the robot center. Y is positive when the object is in front of the robot center.
 */
inline Point2d CalculateConeDisplacement(
	const Point2i cameraCoordinate,
	const Point2i cameraResolution,
	const Point2d cameraFov,
	const Point3d cameraOffset,
	const Point2d cameraAngle) {

	const Point2d inCameraAngleToObject = AngleFromCameraCenter(cameraCoordinate, cameraResolution, cameraFov);
	const Point2d angleFromCameraToObject = cameraAngle + inCameraAngleToObject;

	const double yFromCameraToObject = cameraOffset.z / tan(angleFromCameraToObject.y);
	const double xFromCameraToObject = yFromCameraToObject * tan(angleFromCameraToObject.x);

	const Point2d displacementFromRobotCenter = Point2d(xFromCameraToObject + cameraOffset.x, yFromCameraToObject + cameraOffset.y);
	return displacementFromRobotCenter;
}
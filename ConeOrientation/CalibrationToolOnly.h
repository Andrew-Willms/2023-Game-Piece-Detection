#pragma once

using namespace cv;
using namespace std;
using namespace std::chrono;



inline void DrawConeDetails(Mat& targetImage, const vector<Point2i>& coneContour, const ConeDetails& coneDetails,
	const vector<vector<Point2i>>& cornerGroups, MultiImageWindow& guiWindow) {

	if (coneContour.empty()) {
		guiWindow.AddImage(targetImage, 1, 2, "Contours");
		return;
	}

	// draw circle around centroid
	circle(targetImage, coneDetails.GetCentroidCameraPosition(),
		DistanceBetweenPoints(coneDetails.GetCentroidCameraPosition(), coneDetails.GetTipCameraPosition()) * 0.85, GREEN, 2);

	// draw dots on corner group points
	int colorIndex = 0;
	for (const vector<Point2i>& cornerGroup : cornerGroups) {
		for (const Point2i point : cornerGroup) {
			drawMarker(targetImage, point, GetColorByIndex(colorIndex));
		}
		colorIndex++;
	}

	// outline cone
	DrawContour(targetImage, coneContour, MAGENTA);

	// draw line from centroid to tip
	line(targetImage, coneDetails.GetTipCameraPosition(), coneDetails.GetCentroidCameraPosition(), RED);

	// write cone details on screen
	const string text = "X:" + to_string(coneDetails.GetCentroidPosition().x) +
		", Y:" + to_string(coneDetails.GetCentroidPosition().y) +
		", A:" + to_string(coneDetails.GetAngle() * 180 / PI);

	putText(targetImage, text, Point2i(20, 45), 0, 0.75, GREEN);

	guiWindow.AddImage(targetImage, 1, 2, "Contours");
}

#ifdef FROM_WEBCAM
inline vector<VideoCapture> CreateWebCamVideoCaptures() {

	vector<VideoCapture> videoCaptures = vector<VideoCapture>();

	for (int i = 0; i < 4; i++) {
		videoCaptures.emplace_back(i);
	}
}
#endif

#ifdef FROM_FILE
inline vector<Mat> ReadAllFrames() {

	vector<Mat> frames = vector<Mat>();

	Mat frame;
	VideoCapture videoCapture(FROM_FILE);
	videoCapture.read(frame);
	videoCapture.read(frame);

	while (frame.rows != 0) {
		frames.push_back(frame.clone());
		videoCapture.read(frame);
	}

	return frames;
}
#endif

#ifdef FROM_WEBCAM
inline void ReadFromCameraOrRedIfError(vector<VideoCapture>& videoCaptures, const Parameters parameters, Mat& targetImage) {

	videoCaptures[parameters.CameraId].read(targetImage);

	if (targetImage.rows == 0) {
		targetImage = Mat(parameters.CameraResolution.y, parameters.CameraResolution.x, CV_8UC3, RED);
	}
}
#endif

#ifdef FROM_FILE
inline void ChangeFrameBasedOnPlayStateAndKeyPresses(bool& play, int& currentFrameIndex, const int framesInFile) {

	const int keyPressed = waitKey(1);

	// space
	if (keyPressed == 32) {
		play = !play;
	}

	if (currentFrameIndex == framesInFile) {
		play = false;
	}

	if (play) {
		currentFrameIndex = min(framesInFile - 1, currentFrameIndex + 1);
		return;
	}

	// >
	if (keyPressed == 46 && currentFrameIndex + 1 < framesInFile) {
		currentFrameIndex = min(framesInFile - 1, currentFrameIndex + 1);

		// <
	} else if (keyPressed == 44 && currentFrameIndex > 0) {
		currentFrameIndex = max(0, currentFrameIndex - 1);

		// '
	} else if (keyPressed == 39 && currentFrameIndex + 1 < framesInFile) {
		currentFrameIndex = min(framesInFile - 1, currentFrameIndex + 5);

		// ;
	} else if (keyPressed == 59 && currentFrameIndex > 0) {
		currentFrameIndex = max(0, currentFrameIndex - 5);

		// ]
	} else if (keyPressed == 93 && currentFrameIndex + 1 < framesInFile) {
		currentFrameIndex = min(framesInFile - 1, currentFrameIndex + 25);

		// [
	} else if (keyPressed == 91 && currentFrameIndex > 0) {
		currentFrameIndex = max(0, currentFrameIndex - 25);
	}
}
#endif
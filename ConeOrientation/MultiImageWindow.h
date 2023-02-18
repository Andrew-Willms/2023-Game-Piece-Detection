#pragma once

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace std;
using namespace cv;



class MultiImageWindow {

		string WindowTitle;

		uint TotalImageWidth = 300;
		uint TotalImageHeight = 200;
		
		uint ColumnCount;
		uint RowCount;

		Mat WholeImage;
		vector<vector<Mat>> SubImages;
		vector<vector<String>> SubImageNames;

		Mat GetImagePortion(const uint column, const uint row) const {

			if (row >= RowCount) {
				return Mat();
			}

			if (column >= ColumnCount) {
				return Mat();
			}

			return Mat(WholeImage, Rect(GetSubImageStartX(column), GetSubImageStartY(row), GetSubImageWidth(), GetSubImageHeight()));
		}

		uint GetSubImageWidth() const {
			return TotalImageWidth / ColumnCount;
		}

		uint GetSubImageHeight() const {
			return TotalImageHeight / RowCount;
		}

		uint GetSubImageStartX(const uint column) const {
			return column * GetSubImageWidth();
		}

		uint GetSubImageEndX(const uint column) const {
			return (column + 1) * GetSubImageWidth();
		}

		uint GetSubImageStartY(const uint row) const {
			return row * GetSubImageHeight();
		}

		uint GetSubImageEndY(const uint row) const {
			return (row + 1) * GetSubImageHeight();
		}

		static Mat ConvertedTo3Channel(const Mat& image) {

			switch (image.channels()) {

				case 1: {
					Mat threeChannelImage;
					cvtColor(image, threeChannelImage, COLOR_GRAY2BGR);
					return threeChannelImage;
				}

				case 3:
					return image;

				default:
					return Mat(100, 100, CV_8UC3, BLACK);
			}
		}

		Mat ResizedToSubImage(const Mat& image) const {

			const int currentWidth = image.cols;
			const int currentHeight = image.rows;

			const double horizontalScaleFactor = GetSubImageWidth() / (double)currentWidth;
			const double verticalScaleFactor = GetSubImageHeight() / (double)currentHeight;
			const double scaleFactor = min(horizontalScaleFactor, verticalScaleFactor);

			Mat imageResized;
			resize(image, imageResized, Size(), scaleFactor, scaleFactor);

			const int horizontalImagePadding = GetSubImageWidth() - imageResized.cols;
			const int verticalImagePadding = GetSubImageHeight() - imageResized.rows;

			Mat imagePadded;
			copyMakeBorder(imageResized, imagePadded, 0, verticalImagePadding, 0, horizontalImagePadding, BORDER_CONSTANT, BLACK);

			return imagePadded;
		}

		void PrintSubtitle(const uint column, const uint row, const string& subtitle) {

			putText(WholeImage, subtitle, Point(GetSubImageStartX(column), GetSubImageStartY(row) + 18), 0, 0.75, MAGENTA);
		}

	public:

		MultiImageWindow(const string& windowTitle, const uint columnCount, const uint rowCount) {

			WindowTitle = windowTitle;

			ColumnCount = columnCount;
			RowCount = rowCount;

			const Mat placeHolderSubImage = Mat(10, 10, CV_8UC3, GREEN);
			SubImages = vector<vector<Mat>>(columnCount, vector<Mat>(rowCount, placeHolderSubImage));
			SubImageNames = vector<vector<string>>(columnCount, vector<string>(rowCount, ""));
		}

		void AddImage(const Mat& image, const uint column, const uint row, const string& subtitle = "") {

			if (row >= RowCount) {
				return;
			}

			if (column >= ColumnCount) {
				return;
			}

			Mat copy;
			image.copyTo(copy);

			SubImages[column][row] = copy;
			SubImageNames[column][row] = subtitle;
		}

		void Show(const int windowWidth, const int windowHeight) {

			TotalImageWidth = max(windowWidth, 200);
			TotalImageHeight = max(windowHeight, 200);

			WholeImage = Mat(TotalImageHeight, TotalImageWidth, CV_8UC3, BLACK);

			for (uint column = 0; column < ColumnCount; column++) {
				for(uint row = 0; row < RowCount; row++) {

					Mat image = SubImages[column][row];

					const Mat threeChannelImage = ConvertedTo3Channel(image);
					const Mat imageResized = ResizedToSubImage(threeChannelImage);

					Mat imagePortion = GetImagePortion(column, row);
					imageResized.copyTo(imagePortion);

					PrintSubtitle(column, row, SubImageNames[column][row]);
				}
			}

			imshow("WindowTitle", WholeImage);
		}

};
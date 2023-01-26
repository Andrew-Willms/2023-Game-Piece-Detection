#pragma once

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace std;
using namespace cv;



class MultiImageWindow {

		string WindowTitle;

		uint TotalImageWidth;
		uint TotalImageHeight;
		
		uint ColumnCount;
		uint RowCount;

		Mat WholeImage;

		Mat GetSubImage(const uint column, const uint row) const {

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
			const double verticalScaleFactor = GetSubImageWidth() / (double)currentHeight;
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

		MultiImageWindow(const string& windowTitle, const uint totalImageWidth, const uint totalImageHeight, const uint columnCount, const uint rowCount) {

			WindowTitle = windowTitle;

			TotalImageWidth = totalImageWidth;
			TotalImageHeight = totalImageHeight;

			ColumnCount = columnCount;
			RowCount = rowCount;

			WholeImage = Mat(totalImageHeight, totalImageWidth, CV_8UC3, BLACK);
		}

		void AddImage(const Mat& image, const uint column, const uint row, const string& subtitle = "") {

			if (row >= RowCount) {
				return;
			}

			if (column >= ColumnCount) {
				return;
			}

			const Mat threeChannelImage = ConvertedTo3Channel(image);
			const Mat imageResized = ResizedToSubImage(threeChannelImage);

			Mat subImage = GetSubImage(column, row);
			imageResized.copyTo(subImage);

			PrintSubtitle(column, row, subtitle);
		}

		void Show() const {
			imshow("WindowTitle", WholeImage);
		}

};
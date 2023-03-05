#pragma once

#include <opencv2/core/types.hpp>

using namespace cv;
using namespace std;



const Scalar BLACK = Scalar(0, 0, 0);
const Scalar WHITE = Scalar(255, 255, 255);
const Scalar BLUE = Scalar(255, 0, 0);
const Scalar GREEN = Scalar(0, 255, 0);
const Scalar RED = Scalar(0, 0, 255);
const Scalar MAGENTA = Scalar(255, 0, 255);

const vector<Scalar> COLORS = vector<Scalar>{
	BLACK, WHITE, BLUE, GREEN, RED, MAGENTA
};

Scalar GetColorByIndex(int index) {

	index = index % COLORS.size();

	return COLORS[index];
}
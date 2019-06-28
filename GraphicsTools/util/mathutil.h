// =================
// common/mathutil.h
// =================

#ifndef _INCLUDE_COMMON_MATHUTIL_H_
#define _INCLUDE_COMMON_MATHUTIL_H_

#include "common/common.h"

void MaurerEDT(int* img, int w, int h);

// https://en.wikipedia.org/wiki/Simple_linear_regression#Fitting_the_regression_line
// http://paulbourke.net/miscellaneous/interpolation/
class BestFitLine
{
public:
	BestFitLine();
	void AddPoint(float x, float y);
	float GetLine(float& a, float& b) const; // line is y = a + b*x, returns regression coefficient (i.e. "fit")

private:
	float m_sum_x;
	float m_sum_y;
	float m_sum_xx;
	float m_sum_yy;
	float m_sum_xy;
	float m_sum_1;
};

#endif // _INCLUDE_COMMON_MATHUTIL_H_
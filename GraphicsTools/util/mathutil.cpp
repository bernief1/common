// ===================
// common/mathutil.cpp
// ===================

#include "mathutil.h"

#include "vmath/vmath_common.h"

// Euclidean Distance Transform (EDT)
// ----------------------------------------------------------------
// Algorithm based on Ricardo Fabbri's implementation of Maurer EDT
// http://www.lems.brown.edu/~rfabbri/stuff/fabbri-EDT-survey-ACMCSurvFeb2008.pdf
// http://tc18.liris.cnrs.fr/subfields/distance_skeletons/DistanceTransform.pdf
// http://www.lems.brown.edu/vision/people/leymarie/Refs/CompVision/DT/DTpaper.pdf
// http://www.comp.nus.edu.sg/~tants/jfa/i3d06.pdf (jump flooding)
// http://www.comp.nus.edu.sg/~tants/jfa/rong-guodong-phd-thesis.pdf
// http://en.wikipedia.org/wiki/Distance_transform

static void MaurerEDT_Horizontal(int* img, int w, int y)
{
	int* row = &img[y*w];
	if (row[0] != 0)
		row[0] = w;
	for (int x = 1; x < w; x++) {
		if (row[x] != 0)
			row[x] = row[x - 1] + 1;
	}
	for (int x = w - 2; x >= 0; x--) {
		if (row[x] > row[x + 1] + 1)
			row[x] = row[x + 1] + 1;
	}
	for (int x = 0; x < w; x++) {
		if (row[x] >= w)
			row[x] = -1; // -1 is "infinity"
		else
			row[x] = row[x]*row[x]; // distance squared
	}
}

static inline bool MaurerEDT_Remove(int du, int dv, int dw, int u, int v, int w)
{
	const int64 a = v - u; // these need to be 64-bit ints so the calculation below doesn't overflow
	const int64 b = w - v;
	const int64 c = w - u;
	return (c*dv - b*du - a*dw) > a*b*c;
}

static void MaurerEDT_Vertical(int* img, int w, int h, int x, int* G, int* H)
{
	int l1 = -1;
	int l2 = 0;
	int* col = img + x;
	for (int y = 0; y < h; y++) {
		const int fi = *col; col += w;
		if (fi != -1) {
			while (l1 > 0 && MaurerEDT_Remove(G[l1-1], G[l1], fi, H[l1-1], H[l1], y))
				l1--;
			l1++;
			G[l1] = fi;
			H[l1] = y;
		}
	}
	if (l1 == -1)
		return;
	col = img + x;
	for (int y = 0; y < h; y++) {
		int tmp0 = H[l2] - y;
		int tmp1 = G[l2] + tmp0*tmp0;
		while (l2 < l1) {
			const int tmp2 = H[l2+1] - y;
			if (tmp1 <= G[l2+1] + tmp2*tmp2)
				break;
			l2++;
			tmp0 = H[l2] - y;
			tmp1 = G[l2] + tmp0*tmp0;
		}
		*col = tmp1; col += w;
	}
}

void MaurerEDT(int* img, int w, int h)
{
	int* G = new int[h];
	int* H = new int[h];
	for (int y = 0; y < h; y++)
		MaurerEDT_Horizontal(img, w, y);
	for (int x = 0; x < w; x++)
		MaurerEDT_Vertical(img, w, h, x, G, H);
	delete[] G;
	delete[] H;
}

BestFitLine::BestFitLine()
{
	memset(this, 0, sizeof(*this));
}

void BestFitLine::AddPoint(float x, float y)
{
	m_sum_x  += x;
	m_sum_y  += y;
	m_sum_xx += x*x;
	m_sum_yy += y*y;
	m_sum_xy += x*y;
	m_sum_1  += 1.0f;
}

float BestFitLine::GetLine(float& a, float& b) const
{
	if (m_sum_1 >= 2.0f) {
		const float q = 1.0f/m_sum_1;
		const float sxx = m_sum_xx - m_sum_x*m_sum_x*q;
		const float syy = m_sum_yy - m_sum_y*m_sum_y*q;
		const float sxy = m_sum_xy - m_sum_x*m_sum_y*q;
		if (Abs(sxx) > 0.000001f) {
			b = sxy/sxx;
			a = (m_sum_y - b*m_sum_x)*q;
			if (sxx*syy > 0.000001f)
				return sxy/sqrtf(sxx*syy);
			else
				return 1.0f;	
		}
	}
	return a = b = 0.0f;
}
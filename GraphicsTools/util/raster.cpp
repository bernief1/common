// =================
// common/raster.cpp
// =================

/*
TODO
- clean up and expose public interface (input triangles, output array of span data)
- fix corner cases
- automatic testing (watertight, etc.), test zero-area triangles, slivers, degenerates, CCW vs CW, 
- parameter interpolation (V4's)
- scissor clipping
- antialiasing? (possible to do perfect antialiasing if guaranteed uniform coverage)
- homogeneous coords, viewport clipping, z-buffer
- SSE / vector optimize
NOTE
- floating-point texture coordinates (T) and positions (P) are pixel CORNERS, i.e. [0.5,0.5] is the center of the topleft pixel
- pixels are considered to be inside a triangle if their center is inside the triangle
- this allows one to draw a quad of dimensions [w,h] at origin [x,y] by passing positions [x,y,x+w,y+h]
- to convert from INTEGER pixel location to floating-point coordinate: cast to float and add 0.5f
- to convert a span [a,b] from floating-point coordinates to integer pixel locations: x0 = ceil(a - 0.5f), x1 = ceil(b - 0.5f)
    (then covered pixels (x) will be x0 <= x < x1)
*/

#include "raster.h"

namespace raster {

class Tri
{
public:
	enum { Num_verts = 3 };
	inline Tri() {}

	//    0
	//   / \
	//  /   \
	// 1-----2

	inline Tri(const Vert &v0, const Vert &v1, const Vert &v2)
	{
		m_v[0] = v0;
		m_v[1] = v1;
		m_v[2] = v2;
	}

	inline Vec4V_out GetT(Vec2V_arg p) const
	{
		const Vec2V v = p - m_v[2].m_p;
		return m_v[2].m_t + m_DTDX*v.x() + m_DTDY*v.y();
	}

	inline Vec4V_out GetT(int i, int j) const // get interpolator value at center of pixel [i,j]
	{
		const Vec2V p((float)i + 0.5f, (float)j + 0.5f);
		return GetT(p);
	}

	void Setup(int w, int h)
	{
		const Vec2V scale((float)w, (float)h);
		m_v[0].m_p *= scale;
		m_v[1].m_p *= scale;
		m_v[2].m_p *= scale;

		// rotate so that vertex A (p0) is the top
		if (m_v[0].m_p.y() > m_v[1].m_p.y() || m_v[0].m_p.y() >= m_v[2].m_p.y()) {
			Wind1();
			if (m_v[0].m_p.y() > m_v[1].m_p.y() || m_v[0].m_p.y() >= m_v[2].m_p.y())
				Wind1();
		}

		// flip vertices B,C so that triangle is counter-clockwise
		const Vec2V dAB = m_v[1].m_p - m_v[0].m_p;
		const Vec2V dAC = m_v[2].m_p - m_v[0].m_p;
		if (Cross(dAB, dAC) > 0.0f)
			std::swap(m_v[1], m_v[2]);

		const Vec2V av = m_v[0].m_p - m_v[2].m_p; // p0 - p2
		const Vec2V bv = m_v[1].m_p - m_v[2].m_p; // p1 - p2
		const Vec4V at = m_v[0].m_t - m_v[2].m_t; // t0 - t2
		const Vec4V bt = m_v[1].m_t - m_v[2].m_t; // t1 - t2

		const ScalarV area_x2 = Cross(bv, av); // triangle area*2
		if (area_x2 > 0.0f)
			m_q = Recip(area_x2);
		else
			m_q = ScalarV(V_ZERO);

		m_DTDX = m_q*(bt*av.y() - at*bv.y());
		m_DTDY = m_q*(at*bv.x() - bt*av.x());
	}

	Vert    m_v[Num_verts]; // vertices (x,y position + 4D interpolator)
	ScalarV m_q; // 1/(area*2) (this value is scalar)
	Vec4V   m_DTDX; // interpolator x-gradient
	Vec4V   m_DTDY; // interpolator y-gradient

private:
	inline void Wind1()
	{
		Vert v3 = m_v[0]; m_v[0] = m_v[1]; m_v[1] = m_v[2]; m_v[2] = v3;
	}
};

// ================================================================================================

class Span
{
public:
	float m_x0;
	float m_x1;
	int m_y;
	RASTER_DEBUG_ONLY(Vec4V m_debugColor);
};

class SpanList
{
public:
	inline SpanList() { m_count = 0; }

	inline void AddSpan(float x0, float x1, int y RASTER_DEBUG_ONLY(, Vec4V_arg color = Vec4V(V_WAXIS)))
	{
		DEBUG_ASSERT(m_count < COUNT_MAX);
		DEBUG_ASSERT(m_count == 0 || m_spans[m_count - 1].m_y < y);
		Span& s = m_spans[m_count++];
		s.m_x0 = x0;
		s.m_x1 = x1;
		s.m_y = y;
		RASTER_DEBUG_ONLY(s.m_debugColor = color);
	}

	enum { COUNT_MAX = 4096 };
	Span m_spans[COUNT_MAX];
	int  m_count;
};

static void BuildSpansInternal(SpanList& spans,
	const Vert& L0, // top left
	const Vert& R0, // top right
	int         y0, // top
	const Vert& L1, // bottom left
	const Vert& R1, // bottom right
	int         y1) // bottom
{
	//          L0
	//          /
	//         /    R0
	//        /      \
	//       +--------+ - - - - y0
	//      /          \
	//     /            \
	//    /              \
	//   +----------------+ - - y1
	//  /                  \
	// L1                   \
	//                       \
	//                       R1

	if (L1.m_p.y() == L0.m_p.y() || R1.m_p.y() == R0.m_p.y()) // TODO - REMOVE (check outside this function)
		return;
	DEBUG_ASSERT(L1.m_p.y() != L0.m_p.y());
	DEBUG_ASSERT(R1.m_p.y() != R0.m_p.y());
	const float dxdL = (L1.m_p.xf() - L0.m_p.xf())/(L1.m_p.yf() - L0.m_p.yf()); // left gradient
	const float dxdR = (R1.m_p.xf() - R0.m_p.xf())/(R1.m_p.yf() - R0.m_p.yf()); // right gradient
	const float xL0 = L0.m_p.xf();
	const float xR0 = R0.m_p.xf();
	for (int y = y0; y < y1; y++) {
		const float yf = (float)y + 0.5f;
		const float xL = xL0 + (yf - L0.m_p.yf())*dxdL; // TODO - fix single-pixel errors due to gradient being calculated outside of loop
		const float xR = xR0 + (yf - R0.m_p.yf())*dxdR;
		const float x0 = Ceiling(xL - 0.5f);
		const float x1 = Ceiling(xR - 0.5f);
		if (x0 < x1)
			spans.AddSpan(x0,x1,y);
	}
}

static void BuildSpans(SpanList& spans, const Tri& tri)
{
	// rasterize pixel centers (e.g. rendering triangles into a framebuffer)
	if (tri.m_v[1].m_p.y() <= tri.m_v[2].m_p.y())
	{
		//     A - - - y0
		//    /|
		//   / |     (top)
		//  /  |
		// B---+ - - - y1,y2
		//  \  |
		//   \ |     (bottom)
		//    \|
		//     C - - - y3

		const int y0 = (int)Ceiling(tri.m_v[0].m_p.yf() - 0.5f);
		const int y1 = (int)Ceiling(tri.m_v[1].m_p.yf() - 0.5f);
		const int y2 = y1;
		const int y3 = (int)Ceiling(tri.m_v[2].m_p.yf() - 0.5f);

		BuildSpansInternal(spans, tri.m_v[0], tri.m_v[0], y0,tri.m_v[1], tri.m_v[2], y1); // top
		BuildSpansInternal(spans, tri.m_v[1], tri.m_v[0], y2,tri.m_v[2], tri.m_v[2], y3); // bottom
	}
	else
	{
		// A - - - - y0
		// |\
		// | \     (top)
		// |  \
		// +---C - - y1,y2
		// |  /
		// | /     (bottom)
		// |/
		// B - - - - y3

		const int y0 = (int)Ceiling(tri.m_v[0].m_p.yf() - 0.5f);
		const int y1 = (int)Ceiling(tri.m_v[2].m_p.yf() - 0.5f);
		const int y2 = y1;
		const int y3 = (int)Ceiling(tri.m_v[1].m_p.yf() - 0.5f);

		BuildSpansInternal(spans, tri.m_v[0], tri.m_v[0], y0,tri.m_v[1], tri.m_v[2], y1); // top
		BuildSpansInternal(spans, tri.m_v[0], tri.m_v[2], y2,tri.m_v[1], tri.m_v[1], y3); // bottom
	}
}

static void BuildSpansCoverage(SpanList& spans, const Tri& tri)
{
	// pixel "coverage" (e.g. determining which tiles overlap with a triangle)
#if RASTER_DEBUG
	// debug colors ...
	const Vec4V spanColor_TRIVIAL(0.8f, 0.8f, 0.8f, 1.0f); // white
	const Vec4V spanColor_TOP    (0.8f, 0.0f, 0.0f, 1.0f); // red
	const Vec4V spanColor_TOP1   (0.8f, 0.8f, 0.0f, 1.0f); // red-green
	const Vec4V spanColor_UPPER  (0.4f, 0.4f, 0.4f, 1.0f); // red-gray (red)
	const Vec4V spanColor_MIDDLE (0.0f, 0.8f, 0.0f, 1.0f); // green
	const Vec4V spanColor_LOWER  (0.4f, 0.4f, 0.5f, 1.0f); // blue-gray
	const Vec4V spanColor_BOTTOM1(0.0f, 0.8f, 0.8f, 1.0f); // blue-green
	const Vec4V spanColor_BOTTOM (0.0f, 0.0f, 0.8f, 1.0f); // blue
#endif // RASTER_DEBUG

	float Ax = tri.m_v[0].m_p.xf(), Ay = tri.m_v[0].m_p.yf();
	float Bx = tri.m_v[1].m_p.xf(), By = tri.m_v[1].m_p.yf();
	float Cx = tri.m_v[2].m_p.xf(), Cy = tri.m_v[2].m_p.yf();

	//DEBUG_ASSERT(IsNormal(Ax) && IsNormal(Ay)); // INF or NaN input are not supported, also denormals are not supported
	//DEBUG_ASSERT(IsNormal(Bx) && IsNormal(By));
	//DEBUG_ASSERT(IsNormal(Cx) && IsNormal(Cy));

	float Ay0 = Floor(Ay);
	int iy = (int)Ay0; // index of first span

	float xmin = Min(Ax, Bx, Cx); // triangle bounds
	float xmax = Max(Ax, Bx, Cx);
	const float ymax = Max(By, Cy);

	// first check for trivial cases
	const float range = Min(Ceiling(xmax) - Floor(xmin), Ceiling(ymax) - Ay0);
	if (range == 0.0f) // entire triangle spans 0 rows or columns - zero coverage
		return; // done
	else if (range == 1.0f) { // entire triangle spans 1 row or column
		while (Ay0 < ymax) {
			spans.AddSpan(xmin, xmax, iy RASTER_DEBUG_ONLY(, spanColor_TRIVIAL));
			iy++, Ay0++;
		}
		return; // done
	}
	float y1 = Ay0 + 1.0f; // lower edge of first span

	const bool flip = (By > Cy);
	if (flip) {
		Ax = -Ax; // negate x-coords
		Bx = -Bx;
		Cx = -Cx;
		xmin = -xmin;
		xmax = -xmax;
		std::swap(Bx, Cx); // swap B and C
		std::swap(By, Cy);
		std::swap(xmin, xmax); // swap xmin and xmax
	}

	DEBUG_ASSERT(By <= Cy);

	const float dAB = (Bx - Ax)/(By - Ay); // divide by zero is possible, but the results will not be used ..
	const float dBC = (Cx - Bx)/(Cy - By);
	const float dCA = (Ax - Cx)/(Ay - Cy);

	const float dAB_pos = Max(0.0f, dAB);
	const float dBC_pos = Max(0.0f, dBC);
	const float dCA_pos = Max(0.0f, dCA);
	const float dAB_neg = Min(0.0f, dAB);
	const float dBC_neg = Min(0.0f, dBC);
	const float dCA_neg = Min(0.0f, dCA);

	const float By0 = Floor(By);
	const float Cy1_sub1 = Ceiling(Cy) - 1.0f;
	float By1 = Ceiling(By);

	// TOP SPAN ....... [floor(Ay),   floor(Ay)+1]
	// UPPER SPANS .... [floor(Ay)+1, floor(By)  ] 
	// MIDDLE SPAN .... [floor(By),   ceil (By)  ]
	// LOWER SPANS .... [ceil (By),   ceil (Cy)-1]
	// BOTTOM SPANS ... [ceil (Cy)-1, ceil (Cy)  ]

	if (By1 == y1) // top half is 1 span [Ay0..By1], will include or touch (A) and (B) but will not touch (C)
	{
		DEBUG_ASSERT(IsFinite(dBC_neg));
		DEBUG_ASSERT(IsFinite(dCA_pos));
		DEBUG_ASSERT(y1 == (float)(iy + 1));

		const float x0 = (y1 - By)*dBC_neg + Min(Ax, Bx);
		const float x1 = (y1 - Ay)*dCA_pos + Ax;

		DEBUG_ASSERT(xmin <= x0 && x0 < x1 && x1 <= xmax);

		spans.AddSpan(x0, x1, iy RASTER_DEBUG_ONLY(, spanColor_TOP1));
		iy++, y1++;

		By1 = By0; // skip middle span
	}
	else if (By1 > y1) // top half is multiple spans ..
	{
		// top span [Ay0..Ay0+1], will include or touch (A) on top but will not touch (B) or (C)
		{
			DEBUG_ASSERT(IsFinite(dAB_neg));
			DEBUG_ASSERT(IsFinite(dCA_pos));
			DEBUG_ASSERT(y1 == (float)(iy + 1));

			const float x0 = (y1 - Ay)*dAB_neg + Ax;
			const float x1 = (y1 - Ay)*dCA_pos + Ax;

			DEBUG_ASSERT(xmin <= x0 && x0 < x1 && x1 <= xmax);

			spans.AddSpan(x0, x1, iy RASTER_DEBUG_ONLY(, spanColor_TOP));
			iy++, y1++;
		}

		while (y1 <= By0) // upper spans [Ay0+1..By0], may touch (B) on bottom but will not touch (A) ... may touch (C) if Cy=By is an integer
		{
			DEBUG_ASSERT(IsFinite(dAB));
			DEBUG_ASSERT(IsFinite(dCA));
			DEBUG_ASSERT(y1 == (float)(iy + 1));

			const float x0 = ((y1 - By)*dAB - dAB_pos) + Bx; // must = Bx exactly when y = By and dAB <= 0
			const float x1 = ((y1 - Cy)*dCA - dCA_neg) + Cx; // must = Cx exactly when y = Cy and dCA >= 0

			DEBUG_ASSERT(xmin <= x0 && x0 < x1 && x1 <= xmax);

			spans.AddSpan(x0, x1, iy RASTER_DEBUG_ONLY(, spanColor_UPPER));
			iy++, y1++;
		}
	}

	float y0 = y1 - 1.0f; // now we track the upper edge of the span

	if (By0 == Cy1_sub1) // bottom half is 1 span [By0..Cy1], will include or touch (B) and (C) but will not touch (A)
	{
		DEBUG_ASSERT(IsFinite(dAB_pos));
		DEBUG_ASSERT(IsFinite(dCA_neg));
		DEBUG_ASSERT(y0 == (float)iy);

		const float x0 = (y0 - By)*dAB_pos + Min(Bx, Cx);
		const float x1 = (y0 - Cy)*dCA_neg + Cx;

		DEBUG_ASSERT(xmin <= x0 && x0 < x1 && x1 <= xmax);

		spans.AddSpan(x0, x1, iy RASTER_DEBUG_ONLY(, spanColor_BOTTOM1));
		//iy++, y0++;
	}
	else if (By0 < Cy1_sub1) // bottom half is multiple spans ..
	{
		if (By0 < By1) // middle span [By0..By1], will include or touch (B) on top or bottom but will not touch (A) or (C) 
		{
			DEBUG_ASSERT(IsFinite(dAB_pos));
			DEBUG_ASSERT(IsFinite(dBC_neg));
			DEBUG_ASSERT(IsFinite(dCA));
			DEBUG_ASSERT(y0 == (float)(iy + 0));
			DEBUG_ASSERT(y1 == (float)(iy + 1));
			DEBUG_ASSERT(dAB_pos*dBC_neg == 0.0f); // NOTE: dAB_pos and dBC_neg cannot simultaneously be non-zero due to CCW winding

			const float x0  = ((y0 - By)*dAB_pos + (y1 - By)*dBC_neg) + Bx;
			const float x1  = ((y0 - Ay)*dCA     +           dCA_pos) + Ax; // all four of these (x1,x1b,x1c,x1d) are valid ...
			const float x1b = ((y1 - Ay)*dCA     -           dCA_neg) + Ax;
			const float x1c = ((y0 - Cy)*dCA     +           dCA_pos) + Cx;
			const float x1d = ((y1 - Cy)*dCA     -           dCA_neg) + Cx;

			DEBUG_ASSERT(xmin <= x0 && x0 < x1 && x1 <= xmax);

			spans.AddSpan(x0, x1, iy RASTER_DEBUG_ONLY(, spanColor_MIDDLE));
			iy++, y0++;
		}

		while (y0 < Cy1_sub1) // lower spans [By1..Cy1-1], may touch (B) on top but will not touch (C) ... may touch (A) if Ay=By is an integer
		{
			DEBUG_ASSERT(IsFinite(dBC));
			DEBUG_ASSERT(IsFinite(dCA));
			DEBUG_ASSERT(y0 == (float)iy);

			const float x0 = ((y0 - By)*dBC + dBC_neg) + Bx; // must = Bx exactly when y = By and dBC >= 0
			const float x1 = ((y0 - Ay)*dCA + dCA_pos) + Ax; // must = Ax exactly when y = Ay and dCA <= 0

			DEBUG_ASSERT(xmin <= x0 && x0 < x1 && x1 <= xmax);

			spans.AddSpan(x0, x1, iy RASTER_DEBUG_ONLY(, spanColor_LOWER));
			iy++, y0++;
		}

		// bottom span [Cy1-1..Cy1], will include or touch (C) on bottom but will not touch (A) or (B)
		{
			DEBUG_ASSERT(IsFinite(dBC_pos));
			DEBUG_ASSERT(IsFinite(dCA_neg));
			DEBUG_ASSERT(y0 == (float)iy);

			const float x0 = (y0 - Cy)*dBC_pos + Cx; // must = Cx exactly when y = Cy
			const float x1 = (y0 - Cy)*dCA_neg + Cx; // must = Cx exactly when y = Cy

			DEBUG_ASSERT(xmin <= x0 && x0 < x1 && x1 <= xmax);

			spans.AddSpan(x0, x1, iy RASTER_DEBUG_ONLY(, spanColor_BOTTOM));
			//iy++, y0++;
		}
	}
	if (flip) { // negate and swap spans
		for (int i = 0; i < spans.m_count; i++) {
			Span& s = spans.m_spans[i];
			const float x = -s.m_x0;
			s.m_x0 = -s.m_x1;
			s.m_x1 = x;
		}
	}
}

void Rasterize(Vec4V* image, int w, int h, const Vert& v0, const Vert& v1, const Vert& v2, Vec4V_arg blend, bool coverage)
{
	// TODO -- don't construct spans in memory - instead, render each span immediately
	Tri tri(v0, v1, v2);
	tri.Setup(w, h);
	SpanList spans;
	if (coverage)
		BuildSpansCoverage(spans, tri);
	else
		BuildSpans(spans, tri);
	for (int i = 0; i < spans.m_count; i++) {
		const Span& s = spans.m_spans[i];
		if (s.m_y >= 0 && s.m_y < h) {
			const int x0 = Max((int)Floor(s.m_x0), 0);
			const int x1 = Min((int)Ceiling(s.m_x1), w);
			Vec4V t = tri.GetT(x0, s.m_y);
			for (int x = x0; x < x1; x++) {
				Vec4V& dst = image[x + s.m_y*w];
				dst += (t - dst)*blend;
				t += tri.m_DTDX;
			}
		}
	}
}

} // namespace raster
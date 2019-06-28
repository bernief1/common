// ===============
// common/raster.h
// ===============

#ifndef _INCLUDE_COMMON_RASTER_H_
#define _INCLUDE_COMMON_RASTER_H_

#include "common/common.h"

#include "vmath/vmath_common.h"
#include "vmath/vmath_vec4.h"

#define RASTER_DEBUG (0)
#if RASTER_DEBUG
#define RASTER_DEBUG_ONLY(...) __VA_ARGS__
#else
#define RASTER_DEBUG_ONLY(...)
#endif

namespace raster {

class Vert
{
public:
	inline Vert() {}
	inline Vert(Vec2V_arg p, Vec4V_arg t) : m_p(p), m_t(t) {}

	Vec2V m_p; // {x,y} in pixel coordinates
	Vec4V m_t; // interpolators
};

inline Vec2V_out FlipUV(Vec2V_arg uv) { return Vec2V(uv.x(), ScalarV(V_ONE) - uv.y()); }

void Rasterize(Vec4V* image, int w, int h, const Vert& v0, const Vert& v1, const Vert& v2, Vec4V_arg blend = Vec4V(V_ONE), bool coverage = false);

} // namespace raster

#endif // _INCLUDE_COMMON_RASTER_H_
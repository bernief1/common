// ==========================
// common/vmath/vmath_plane.h
// ==========================

#ifndef _INCLUDE_VMATH_PLANE_H_
#define _INCLUDE_VMATH_PLANE_H_

#include "vmath_common.h"
#include "vmath_vec4.h"
#include "vmath_box.h"
#include "vmath_soa.h"

class Plane3V
{
public:
	VMATH_INLINE Plane3V() {}
	VMATH_INLINE Plane3V(Vec3V_arg normal,ScalarV_arg d): m_plane(normal,d) {}
	VMATH_INLINE Plane3V(Vec4V_arg v): m_plane(v) {}

	VMATH_INLINE static const Plane3V ConstructFromPointAndNormal(Vec3V_arg point,Vec3V_arg normal) { return Plane3V(normal,-Dot(point,normal)); }
	VMATH_INLINE static const Plane3V ConstructFrom3Points(Vec3V_arg p1,Vec3V_arg p2,Vec3V_arg p3) { return ConstructFromPointAndNormal(p1,Normalize(Cross(p2 - p1,p3 - p1))); }
	VMATH_INLINE static const Plane3V ConstructFrom3PointsUnnormalized(Vec3V_arg p1,Vec3V_arg p2,Vec3V_arg p3) { return ConstructFromPointAndNormal(p1,Cross(p2 - p1,p3 - p1)); }

	VMATH_INLINE const Plane3V NormalizePlane() const { return m_plane*RecipSqrt(MagSqr(GetNormal())); }

	VMATH_INLINE friend const Plane3V operator -(const Plane3V& a) { return Plane3V(-a.m_plane); }

	VMATH_INLINE Vec4V_out AsVec4V() const { return m_plane; }
	VMATH_INLINE Vec3V_out GetNormal() const { return m_plane.xyz(); }
	VMATH_INLINE ScalarV_out GetDistance() const { return m_plane.w(); }
	
	VMATH_INLINE ScalarV_out GetDistanceToPoint(Vec3V_arg point) const { return Dot(Vec4V(point,1.0f),m_plane); }

	VMATH_INLINE ScalarV_out GetRayIntersectionDistance(Vec3V_arg point, Vec3V_arg dir) const { return -GetDistanceToPoint(point)/Dot(dir, GetNormal()); }
	VMATH_INLINE Vec3V_out GetRayIntersection(Vec3V_arg point, Vec3V_arg dir) const { return point + dir*GetRayIntersectionDistance(point, dir); }

	// TODO -- test
	VMATH_INLINE Vec3V_out GetFurthestBoxCornerOnPositiveSide(const Box3V& box) const { return Select(box.GetMin(),box.GetMax(),Vec3V::BoolV(GetNormal())); }
	VMATH_INLINE Vec3V_out GetFurthestBoxCornerOnNegativeSide(const Box3V& box) const { return Select(box.GetMax(),box.GetMin(),Vec3V::BoolV(GetNormal())); }

private:
	Vec4V m_plane; // xyz=normal,w=distance,where w = -dot(normal,point_on_plane)
};

class Plane3V_SOA4
{
public:
	VMATH_INLINE Plane3V_SOA4() {}
	VMATH_INLINE Plane3V_SOA4(Vec3V_SOA4_arg normal,Vec4V_arg d): m_normal(normal),m_d(d) {}
	
	VMATH_INLINE static const Plane3V_SOA4 ConstructFromPointAndNormal(Vec3V_SOA4_arg point,Vec3V_SOA4_arg normal) { return Plane3V_SOA4(normal,-Dot(point,normal)); }
	VMATH_INLINE static const Plane3V_SOA4 ConstructFrom3Points(Vec3V_SOA4_arg p1,Vec3V_SOA4_arg p2,Vec3V_SOA4_arg p3) { return ConstructFromPointAndNormal(p1,Normalize(Cross(p2 - p1,p3 - p1))); }
	VMATH_INLINE static const Plane3V_SOA4 ConstructFrom3PointsUnnormalized(Vec3V_SOA4_arg p1,Vec3V_SOA4_arg p2,Vec3V_SOA4_arg p3) { return ConstructFromPointAndNormal(p1,Cross(p2 - p1,p3 - p1)); }

	static const Plane3V_SOA4 ConstructFromProjectionMatrix_LTRB(Mat44V_arg proj, Plane3V* out_nearPlane = nullptr, Plane3V* out_farPlane = nullptr);

	VMATH_INLINE const Plane3V_SOA4 NormalizePlane() const { const Vec4V q = RecipSqrt(MagSqr(m_normal)); return Plane3V_SOA4(m_normal*q,m_d*q); }

	VMATH_INLINE friend const Plane3V_SOA4 operator -(const Plane3V_SOA4& a) { return Plane3V_SOA4(-a.m_normal,-a.m_d); }

	VMATH_INLINE Vec3V_SOA4_out GetNormal() const { return m_normal; }
	VMATH_INLINE Vec4V_out GetDistance() const { return m_d; }
	
	VMATH_INLINE const Plane3V GetPlane(unsigned index) const { return Plane3V(m_normal.GetVector(index),ScalarV(m_d[index])); }

	VMATH_INLINE Vec4V_out GetDistanceToPoint(Vec3V_arg point) const { return Dot(point,m_normal) + m_d; }
	VMATH_INLINE Vec4V_out GetDistanceToPoint_SOA(Vec3V_SOA4_arg point) const { return Dot(point,m_normal) + m_d; }

private:
	Vec3V_SOA4 m_normal;
	Vec4V m_d;
};

#endif // _INCLUDE_VMATH_PLANE_H_

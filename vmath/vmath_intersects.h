// ===============================
// common/vmath/vmath_intersects.h
// ===============================

#ifndef _INCLUDE_VMATH_INTERSECTS_H_
#define _INCLUDE_VMATH_INTERSECTS_H_

#include "vmath_common.h"
#include "vmath_vec4.h"
#include "vmath_box.h"
#include "vmath_plane.h"
#include "vmath_sphere.h"

// example libraries:
// https://github.com/speps/TBToolkit/tree/master/TBToolkit/External/MathGeoLib/src/Mesh

// TODO:
// Cylinder3V
// Capsule3V
// Quad3V
// Circle3V (xyzw=plane,xyz=basisx,w=radius)
// Frustum3V
// Triangle2V/3V
// Polygon2V/3V
// Cone2V/3V
// Cube3V (xyz=center,w=size)
// Line2V/3V
// LineSegment2V/3V
// Ray2V/3V
// Hemisphere3V

VMATH_INLINE bool Intersects(Vec2V_arg point,const Box2V& box) { return All(point >= box.GetMin() && point <= box.GetMax()); }
VMATH_INLINE bool Intersects(const Box2V& box1,const Box2V& box2) { return All(box1.GetMax() >= box2.GetMin() && box1.GetMin() <= box2.GetMax()); }
VMATH_INLINE bool Intersects(const Circle2V& circle,const Box2V& box) { const Vec2V v = Max(Vec2V(V_ZERO),Abs(circle.GetCenter() - box.GetCenter()) - box.GetExtent()); return MagSqr(v) <= circle.GetRadiusSqr(); }
VMATH_INLINE bool Intersects(Vec2V_arg point,const Circle2V& circle) { return MagSqr(point - circle.GetCenter()) <= circle.GetRadiusSqr(); }
VMATH_INLINE bool Intersects(const Circle2V& circle1,const Circle2V& circle2) { return MagSqr(circle1.GetCenter() - circle2.GetCenter()) <= Sqr(circle1.GetRadius() + circle2.GetRadius()); }

VMATH_INLINE bool Intersects(Vec3V_arg point,const Box3V& box) { return All(point >= box.GetMin() && point <= box.GetMax()); }
VMATH_INLINE bool Intersects(const Box3V& box1,const Box3V& box2) { return All(box1.GetMax() >= box2.GetMin() && box1.GetMin() <= box2.GetMax()); }
VMATH_INLINE bool Intersects(const Sphere3V& sphere,const Box3V& box) { const Vec3V v = Max(Vec3V(V_ZERO),Abs(sphere.GetCenter() - box.GetCenter()) - box.GetExtent()); return MagSqr(v) <= sphere.GetRadiusSqr(); }
VMATH_INLINE bool Intersects(Vec3V_arg point,const Sphere3V& sphere) { return MagSqr(point - sphere.GetCenter()) <= sphere.GetRadiusSqr(); }
VMATH_INLINE bool Intersects(const Sphere3V& sphere1,const Sphere3V& sphere2) { return MagSqr(sphere1.GetCenter() - sphere2.GetCenter()) <= Sqr(sphere1.GetRadius() + sphere2.GetRadius()); }
VMATH_INLINE bool Intersects(const Plane3V& plane,const Box3V& box)
{
	const Vec3V::BoolV sel(plane.GetNormal());
	const Vec3V b1 = Select(box.GetMax(),box.GetMin(),sel);
	const Vec3V b2 = Select(box.GetMin(),box.GetMax(),sel);
	const ScalarV d1 = plane.GetDistanceToPoint(b1);
	const ScalarV d2 = plane.GetDistanceToPoint(b2);
	return d1*d2 <= 0.0f;
}
VMATH_INLINE uint32 Intersects(const Plane3V_SOA4& plane,const Box3V& box)
{
	const Vec4V::BoolV sel_x(plane.GetNormal().x());
	const Vec4V::BoolV sel_y(plane.GetNormal().y());
	const Vec4V::BoolV sel_z(plane.GetNormal().z());
	const Vec4V min_x = box.GetMin().x();
	const Vec4V min_y = box.GetMin().y();
	const Vec4V min_z = box.GetMin().z();
	const Vec4V max_x = box.GetMax().x();
	const Vec4V max_y = box.GetMax().y();
	const Vec4V max_z = box.GetMax().z();
	const Vec4V b1_x = Select(max_x,min_x,sel_x);
	const Vec4V b1_y = Select(max_y,min_y,sel_y);
	const Vec4V b1_z = Select(max_z,min_z,sel_z);
	const Vec4V b2_x = Select(min_x,max_x,sel_x);
	const Vec4V b2_y = Select(min_y,max_y,sel_y);
	const Vec4V b2_z = Select(min_z,max_z,sel_z);
	const Vec4V d1 = plane.GetDistanceToPoint_SOA(Vec3V_SOA4(b1_x,b1_y,b1_z));
	const Vec4V d2 = plane.GetDistanceToPoint_SOA(Vec3V_SOA4(b2_x,b2_y,b2_z));
	return (d1*d2 <= 0.0f).GetMask();
}

// sphere vs SOA box
template <typename BoxType> VMATH_INLINE uint32 IntersectsBox3V_SOA(const Sphere3V& sphere,const BoxType& box)
{
	typedef typename BoxType::T T;
	StaticAssert(BoxType::NumBoxes > 1);
	const T v = Max(T(V_ZERO),Abs(sphere.GetCenter() - box.GetCenter()) - box.GetExtent());
	return (MagSqr(v) <= sphere.GetRadiusSqr()).GetMask();
}

// box vs SOA box
template <typename BoxType> VMATH_INLINE uint32 IntersectsBox3V_SOA(const Box3V& box1,const BoxType& box2)
{
	StaticAssert(BoxType::NumBoxes > 1);
	return (
		box1.GetMax().x() >= box2.GetMin().x() &&
		box1.GetMax().y() >= box2.GetMin().y() &&
		box1.GetMax().z() >= box2.GetMin().z() &&
		box1.GetMin().x() <= box2.GetMax().x() &&
		box1.GetMin().y() <= box2.GetMax().y() &&
		box1.GetMin().z() <= box2.GetMax().z()).GetMask();
}

VMATH_INLINE bool IntersectsTransformed(const Box3V& box1, Mat34V_arg transform1, const Box3V& box2, Mat34V_arg transform2)
{
	if (!Intersects(box1,MultiplyInvertOrtho(transform2,transform1).TransformBox(box2)))
		return false;
	else if (!Intersects(MultiplyInvertOrtho(transform1,transform2).TransformBox(box1),box2))
		return false;
	return true; // TODO -- separating axis theorem, there are 9 more cases to test
}

// Contains(A,B) means "A contains B"
VMATH_INLINE bool Contains(const Box2V& box1,const Box2V& box2) { return All(box2.GetMin() >= box1.GetMin() && box2.GetMax() <= box1.GetMax()); }
VMATH_INLINE bool Contains(const Circle2V& circle,const Box2V& box) { return MagSqr(Abs(circle.GetCenter() - box.GetCenter()) + box.GetExtent()) <= circle.GetRadiusSqr(); }
VMATH_INLINE bool Contains(const Box2V& box,const Circle2V& circle) { const Vec2V c = circle.GetCenter(),r = circle.GetRadius(); return All(c - r >= box.GetMin() && c + r <= box.GetMax()); }
VMATH_INLINE bool Contains(const Circle2V& circle1,const Circle2V& circle2) { return MagSqr(circle1.GetCenter() - circle2.GetCenter()) <= Sqr(circle1.GetRadius() - circle2.GetRadius()); }

VMATH_INLINE bool Contains(const Box3V& box1,const Box3V& box2) { return All(box2.GetMin() >= box1.GetMin() && box2.GetMax() <= box1.GetMax()); }
VMATH_INLINE bool Contains(const Sphere3V& sphere,const Box3V& box) { return MagSqr(Abs(sphere.GetCenter() - box.GetCenter()) + box.GetExtent()) <= sphere.GetRadiusSqr(); }
VMATH_INLINE bool Contains(const Box3V& box,const Sphere3V& sphere) { const Vec3V c = sphere.GetCenter(),r = sphere.GetRadius(); return All(c - r >= box.GetMin() && c + r <= box.GetMax()); }
VMATH_INLINE bool Contains(const Sphere3V& sphere1,const Sphere3V& sphere2) { return MagSqr(sphere1.GetCenter() - sphere2.GetCenter()) <= Sqr(sphere1.GetRadius() - sphere2.GetRadius()); }

VMATH_INLINE const Box2V Intersection(const Box2V& box1,const Box2V& box2) { return Box2V(Max(box1.GetMin(),box2.GetMin()),Min(box1.GetMax(),box2.GetMax())); }
VMATH_INLINE const Box3V Intersection(const Box3V& box1,const Box3V& box2) { return Box3V(Max(box1.GetMin(),box2.GetMin()),Min(box1.GetMax(),box2.GetMax())); }

bool TestTriangleBoxIntersect();

#endif // _INCLUDE_VMATH_INTERSECTS_H_

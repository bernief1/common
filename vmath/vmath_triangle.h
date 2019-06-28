// =============================
// common/vmath/vmath_triangle.h
// =============================

#ifndef _INCLUDE_VMATH_TRIANGLE_H_
#define _INCLUDE_VMATH_TRIANGLE_H_

#include "vmath_common.h"
#include "vmath_box.h"
#include "vmath_plane.h"
#include "vmath_soa.h"
#include "vmath_vec16.h"

#define ZBUFFER_DEFAULT 1e9f //FLT_MAX

// TODO -- move this out of this file, it's specific to raytracing
#define RAY_PACKET_INDEPENDENT_ORIGINS (0)

#if RAY_PACKET_INDEPENDENT_ORIGINS
#define RAY_PACKET_INDEPENDENT_ORIGINS_ONLY(...) __VA_ARGS__
#define RAY_PACKET_INDEPENDENT_ORIGINS_SWITCH(_if_independent_origins_,_if_common_origin_) _if_independent_origins_
#define RAY_PACKET_ORIGIN_TYPE_SOA4_arg Vec3V_SOA4_arg
#define RAY_PACKET_ORIGIN_TYPE_SOA8_arg Vec3V_SOA8_arg
#define RAY_PACKET_ORIGIN_TYPE_SOA16_arg Vec3V_SOA16_arg
#else
#define RAY_PACKET_INDEPENDENT_ORIGINS_ONLY(...)
#define RAY_PACKET_INDEPENDENT_ORIGINS_SWITCH(_if_independent_origins_,_if_common_origin_) _if_common_origin_
#define RAY_PACKET_ORIGIN_TYPE_SOA4_arg Vec3V_arg
#define RAY_PACKET_ORIGIN_TYPE_SOA8_arg Vec3V_arg
#define RAY_PACKET_ORIGIN_TYPE_SOA16_arg Vec3V_arg
#endif

#define TRIANGLE_RAY_EPSILON 1.0e-8f
#define TRIANGLE_TWOSIDED_DEFAULT false

// http://en.wikipedia.org/wiki/Barycentric_coordinate_system_(mathematics)
VMATH_INLINE Vec3V_out BarycentricCoords(Vec2V_arg point,Vec2V_arg vert1,Vec2V_arg vert2,Vec2V_arg vert3)
{
	const Vec2V pt = point - vert3;
	const Vec2V v1 = vert1 - vert3;
	const Vec2V v2 = vert2 - vert3;
	const Vec2V v3 = vert3 - vert3;
	const ScalarV denom = Recip(Cross(v1,v2));
	const ScalarV b1 = denom*Cross(pt,v2);
	const ScalarV b2 = denom*Cross(v1,pt);
	return Vec3V(b1,b2,ScalarV(V_ONE) - b1 - b2);
}

class Sphere3V;

class Triangle3V
{
public:
	enum { NumTriangles = 1 };
	enum { NumVerts = 3 };

	VMATH_INLINE Triangle3V() {}
	VMATH_INLINE Triangle3V(Vec3V_arg p0,Vec3V_arg p1,Vec3V_arg p2) { m_positions[0] = p0; m_positions[1] = p1; m_positions[2] = p2; }

	VMATH_INLINE const Triangle3V GetIndexed(unsigned index) const // for compatibility with SOA triangles
	{
		DEBUG_ASSERT(index < NumTriangles);
		return *this;
	}

	VMATH_INLINE const Box3V GetBounds() const
	{
		return Box3V(
			Min(m_positions[0],m_positions[1],m_positions[2]),
			Max(m_positions[0],m_positions[1],m_positions[2]));
	}

	VMATH_INLINE ScalarV_out GetArea() const
	{
		const Vec3V edge01 = m_positions[1] - m_positions[0];
		const Vec3V edge12 = m_positions[2] - m_positions[1];
		const Vec3V edge20 = m_positions[0] - m_positions[2];
		const ScalarV a = Mag(edge01);
		const ScalarV b = Mag(edge12);
		const ScalarV c = Mag(edge20);
		const ScalarV s = (a + b + c)*0.5f;
		return Sqrt(s*(s - a)*(s - b)*(s - c));
	}

	VMATH_INLINE Vec3V_out GetNormal() const
	{
		return Normalize(GetWeightedNormal());
	}

	VMATH_INLINE Vec3V_out GetWeightedNormal() const
	{
		const Vec3V edge01 = m_positions[1] - m_positions[0];
		const Vec3V edge02 = m_positions[2] - m_positions[0];
		return Cross(edge01,edge02); // not normalized - normal magnitude is 2x triangle area
	}

	VMATH_INLINE const Plane3V GetWeightedPlane() const { return Plane3V::ConstructFromPointAndNormal(m_positions[0],GetWeightedNormal()); }
	VMATH_INLINE const Plane3V GetPlane() const { return Plane3V::ConstructFromPointAndNormal(m_positions[0],GetNormal()); }

	uint32 IntersectsRay(Vec3V_arg origin,Vec3V_arg dir,float& t,float& u,float& v,bool twosided = TRIANGLE_TWOSIDED_DEFAULT,float epsilon = TRIANGLE_RAY_EPSILON) const;
	uint32 IntersectsRay(Vec3V_arg origin,Vec3V_arg dir,ScalarV& t,bool twosided = TRIANGLE_TWOSIDED_DEFAULT,float epsilon = TRIANGLE_RAY_EPSILON) const;
	uint32 IntersectsRay(RAY_PACKET_ORIGIN_TYPE_SOA4_arg origin,Vec3V_SOA4_arg dir,Vec4V& t,uint32,bool twosided = TRIANGLE_TWOSIDED_DEFAULT,float epsilon = TRIANGLE_RAY_EPSILON) const;
	uint32 IntersectsRay_intrinsic(__m128 origin,__m128 dir_x,__m128 dir_y,__m128 dir_z,__m128& t) const;
#if HAS_VEC8V
	uint32 IntersectsRay(RAY_PACKET_ORIGIN_TYPE_SOA8_arg origin,Vec3V_SOA8_arg dir,Vec8V& t,uint32,bool twosided = TRIANGLE_TWOSIDED_DEFAULT,float epsilon = TRIANGLE_RAY_EPSILON) const;
	uint32 IntersectsRay_intrinsic(__m128 origin,__m256 dir_x,__m256 dir_y,__m256 dir_z,__m256& t) const;
#if HAS_VEC16V
	uint32 IntersectsRay(RAY_PACKET_ORIGIN_TYPE_SOA16_arg origin,Vec3V_SOA16_arg dir,Vec16V& t,uint32,bool twosided = TRIANGLE_TWOSIDED_DEFAULT,float epsilon = TRIANGLE_RAY_EPSILON) const;
#endif // HAS_VEC16V
#endif // HAS_VEC8V

	uint32 IntersectsRaySign(Vec3V_arg origin,Vec3V_arg dir,ScalarV& t,ScalarV& sign,float epsilon = TRIANGLE_RAY_EPSILON) const;

	uint32 IntersectsSphere(const Sphere3V& sphere) const;
	uint32 IntersectsBox(const Box3V& box) const;

	Vec3V m_positions[3];
};

class Quad3V
{
public:
	enum { NumVerts = 4 };

	VMATH_INLINE Quad3V() {}
	VMATH_INLINE Quad3V(Vec3V_arg p0,Vec3V_arg p1,Vec3V_arg p2,Vec3V_arg p3) { m_positions[0] = p0; m_positions[1] = p1; m_positions[2] = p2; m_positions[3] = p3; }

	VMATH_INLINE const Box3V GetBounds() const
	{
		return Box3V(
			Min(m_positions[0],m_positions[1],m_positions[2],m_positions[3]),
			Max(m_positions[0],m_positions[1],m_positions[2],m_positions[3]));
	}

	VMATH_INLINE const Triangle3V GetTriangle(unsigned index) const
	{
		DEBUG_ASSERT(index < 2);
		return Triangle3V(m_positions[0], m_positions[index + 1], m_positions[index + 2]);
	}

	VMATH_INLINE ScalarV_out GetArea() const
	{
		return GetTriangle(0).GetArea() + GetTriangle(1).GetArea();
	}

	VMATH_INLINE Vec3V_out GetNormal() const
	{
		return Normalize(GetWeightedNormal());
	}

	VMATH_INLINE Vec3V_out GetWeightedNormal() const
	{
		const Vec3V edge02 = m_positions[2] - m_positions[0];
		const Vec3V edge13 = m_positions[3] - m_positions[1];
		return Cross(edge02,edge13); // not normalized - normal magnitude relative to quad area
	}

	Vec3V m_positions[NumVerts];
};

template <unsigned NumTriangles> class Triangle3V_SOA_T {};

template <> class Triangle3V_SOA_T<1>
{
public:
	typedef Triangle3V TriangleType;
};

class Triangle3V_SOA4
{
public:
	typedef Vec3V_SOA4 T;
	enum { NumTriangles = T::NumVectors };
	enum { NumVerts = 3 };

	Triangle3V_SOA4() {}
	Triangle3V_SOA4(const Triangle3V tris[NumTriangles]);

	VMATH_INLINE const Triangle3V GetIndexed(unsigned index) const
	{
		DEBUG_ASSERT(index < NumTriangles);
		return Triangle3V(
			m_positions[0].GetVector(index),
			m_positions[1].GetVector(index),
			m_positions[2].GetVector(index));
	}

	VMATH_INLINE const Box3V GetBounds() const
	{
		const T bmin = Min(m_positions[0],m_positions[1],m_positions[2]);
		const T bmax = Max(m_positions[0],m_positions[1],m_positions[2]);
		return Box3V(
			Vec3V(MinElement(bmin.x()),MinElement(bmin.y()),MinElement(bmin.z())),
			Vec3V(MaxElement(bmax.x()),MaxElement(bmax.y()),MaxElement(bmax.z())));
	}

	VMATH_INLINE const Box3V_SOA4 GetBounds_SOA() const
	{
		const T bmin = Min(m_positions[0],m_positions[1],m_positions[2]);
		const T bmax = Max(m_positions[0],m_positions[1],m_positions[2]);
		return Box3V_SOA4(bmin,bmax);
	}

	VMATH_INLINE Vec3V_SOA4_out GetNormal() const
	{
		return Normalize(GetWeightedNormal());
	}

	VMATH_INLINE Vec3V_SOA4_out GetWeightedNormal() const
	{
		const Vec3V_SOA4 edge01 = m_positions[1] - m_positions[0];
		const Vec3V_SOA4 edge02 = m_positions[2] - m_positions[0];
		return Cross(edge01,edge02); // not normalized - normal magnitude is 2x triangle area
	}

	VMATH_INLINE const Plane3V_SOA4 GetWeightedPlane() const { return Plane3V_SOA4::ConstructFromPointAndNormal(m_positions[0],GetWeightedNormal()); }
	VMATH_INLINE const Plane3V_SOA4 GetPlane() const { return Plane3V_SOA4::ConstructFromPointAndNormal(m_positions[0],GetNormal()); }

	uint32 IntersectsSphere(const Sphere3V& sphere) const;
	uint32 IntersectsBox(const Box3V& box) const; // TODO

	uint32 IntersectsRay(Vec3V_arg origin,Vec3V_arg dir,ScalarV& t,bool twosided = TRIANGLE_TWOSIDED_DEFAULT,float epsilon = TRIANGLE_RAY_EPSILON) const;
	uint32 IntersectsRay(RAY_PACKET_ORIGIN_TYPE_SOA4_arg origin,Vec3V_SOA4_arg dir,Vec4V& t,uint32 mask,bool twosided = TRIANGLE_TWOSIDED_DEFAULT,float epsilon = TRIANGLE_RAY_EPSILON) const;
#if HAS_VEC8V
	uint32 IntersectsRay(RAY_PACKET_ORIGIN_TYPE_SOA8_arg origin,Vec3V_SOA8_arg dir,Vec8V& t,uint32 mask,bool twosided = TRIANGLE_TWOSIDED_DEFAULT,float epsilon = TRIANGLE_RAY_EPSILON) const;
#if HAS_VEC16V
	uint32 IntersectsRay(RAY_PACKET_ORIGIN_TYPE_SOA16_arg origin,Vec3V_SOA16_arg dir,Vec16V& t,uint32 mask,bool twosided = TRIANGLE_TWOSIDED_DEFAULT,float epsilon = TRIANGLE_RAY_EPSILON) const;
#endif // HAS_VEC16V
#endif // HAS_VEC8V

	T m_positions[NumVerts];
};

template <> class Triangle3V_SOA_T<4>
{
public:
	typedef Triangle3V_SOA4 TriangleType;
};

#if HAS_VEC8V
class Triangle3V_SOA8
{
public:
	typedef Vec3V_SOA8 T;
	enum { NumTriangles = T::NumVectors };
	enum { NumVerts = 3 };

	Triangle3V_SOA8() {}
	Triangle3V_SOA8(const Triangle3V tris[NumTriangles]);

	VMATH_INLINE const Triangle3V GetIndexed(unsigned index) const
	{
		DEBUG_ASSERT(index < NumTriangles);
		return Triangle3V(
			m_positions[0].GetVector(index),
			m_positions[1].GetVector(index),
			m_positions[2].GetVector(index));
	}

	VMATH_INLINE const Box3V GetBounds() const
	{
		const T bmin = Min(m_positions[0],m_positions[1],m_positions[2]);
		const T bmax = Max(m_positions[0],m_positions[1],m_positions[2]);
		return Box3V(
			Vec3V(MinElement(bmin.x()),MinElement(bmin.y()),MinElement(bmin.z())),
			Vec3V(MaxElement(bmax.x()),MaxElement(bmax.y()),MaxElement(bmax.z())));
	}

	uint32 IntersectsRay(Vec3V_arg origin,Vec3V_arg dir,ScalarV& t,bool twosided = TRIANGLE_TWOSIDED_DEFAULT,float epsilon = TRIANGLE_RAY_EPSILON) const;
	uint32 IntersectsRay(RAY_PACKET_ORIGIN_TYPE_SOA4_arg origin,Vec3V_SOA4_arg dir,Vec4V& t,uint32 mask,bool twosided = TRIANGLE_TWOSIDED_DEFAULT,float epsilon = TRIANGLE_RAY_EPSILON) const;
#if HAS_VEC8V
	uint32 IntersectsRay(RAY_PACKET_ORIGIN_TYPE_SOA8_arg origin,Vec3V_SOA8_arg dir,Vec8V& t,uint32 mask,bool twosided = TRIANGLE_TWOSIDED_DEFAULT,float epsilon = TRIANGLE_RAY_EPSILON) const;
#if HAS_VEC16V
	uint32 IntersectsRay(RAY_PACKET_ORIGIN_TYPE_SOA16_arg origin,Vec3V_SOA16_arg dir,Vec16V& t,uint32 mask,bool twosided = TRIANGLE_TWOSIDED_DEFAULT,float epsilon = TRIANGLE_RAY_EPSILON) const;
#endif // HAS_VEC16V
#endif // HAS_VEC8V

	T m_positions[NumVerts];
};

template <> class Triangle3V_SOA_T<8>
{
public:
	typedef Triangle3V_SOA8 TriangleType;
};
#endif // HAS_VEC8V

#endif // _INCLUDE_VMATH_TRIANGLE_H_

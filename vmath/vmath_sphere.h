// ===========================
// common/vmath/vmath_sphere.h
// ===========================

#ifndef _INCLUDE_VMATH_SPHERE_H_
#define _INCLUDE_VMATH_SPHERE_H_

#include "vmath_common.h"
#include "vmath_vec4.h"
#include "vmath_box.h"
//#include "vmath_soa.h"

class Circle2V
{
public:
	VMATH_INLINE Circle2V() {}
	VMATH_INLINE Circle2V(Vec2V_arg center,ScalarV_arg radius): m_center_radius(center,radius) {}
	VMATH_INLINE Circle2V(Vec2V_arg center,float radius): m_center_radius(center,radius) {}

	VMATH_INLINE static Circle2V Invalid() { return Circle2V(Vec2V(FLT_MAX),-1.0f); }
	VMATH_INLINE bool IsValid() const { return GetRadius() != -1.0f; }

	VMATH_INLINE Vec2V_out GetCenter() const { return m_center_radius.xy(); }
	VMATH_INLINE ScalarV_out GetRadius() const { return m_center_radius.z(); }
	VMATH_INLINE ScalarV_out GetRadiusSqr() const { return (m_center_radius*m_center_radius).z(); }
	VMATH_INLINE const Box2V GetBounds() const { const Vec2V center = GetCenter(); const Vec2V extent = GetRadius(); return Box2V(center - extent,center + extent); }
	VMATH_INLINE const Circle2V ExpandAbsolute(ScalarV_arg expand) const { return Circle2V(GetCenter(), GetRadius() + expand); }
	VMATH_INLINE const Circle2V ExpandRelative(ScalarV_arg expand) const { return Circle2V(GetCenter(), GetRadius()*expand); }

	Vec2V_out GetRandomPointOnCirclePerimeter() const;
	Vec2V_out GetRandomPointInCircleInterior() const;

	static const Circle2V BuildCircleThrough2Points(Vec2V_arg p1,Vec2V_arg p2);
	static const Circle2V BuildCircleThrough3Points(Vec2V_arg p1,Vec2V_arg p2,Vec2V_arg p3);

	static const Circle2V BuildMinimumEnclosingCircle(const Vec2V* points,int count);
#if VMATH_TEST_GEOM
	static const Circle2V BuildMinimumEnclosingCircle_DEBUG(const Vec2V* points,int count,std::vector<int>& debugBoundaryIndices);
#endif // VMATH_TEST_GEOM

private:
	Vec3V m_center_radius; // xy=center,z=radius
};

#define CIRCLE2V_ARGS(v) (v).GetCenter().xf(),(v).GetCenter().yf(),(v).GetRadius().f()

class Sphere3V
{
public:
	VMATH_INLINE Sphere3V() {}
	VMATH_INLINE Sphere3V(Vec3V_arg center,ScalarV_arg radius): m_center_radius(center,radius) {}
	VMATH_INLINE Sphere3V(Vec3V_arg center,float radius): m_center_radius(center,radius) {}

	VMATH_INLINE static const Sphere3V Invalid() { return Sphere3V(Vec3V(FLT_MAX),-1.0f); }
	VMATH_INLINE bool IsValid() const { return GetRadius() != -1.0f; }

	VMATH_INLINE Vec3V_out GetCenter() const { return m_center_radius.xyz(); }
	VMATH_INLINE ScalarV_out GetRadius() const { return m_center_radius.w(); }
	VMATH_INLINE ScalarV_out GetRadiusSqr() const { const ScalarV radius = GetRadius(); return radius*radius; }
	VMATH_INLINE const Box3V GetBounds() const { const Vec3V center = GetCenter(); const Vec3V extent = GetRadius(); return Box3V(center - extent,center + extent); }
	VMATH_INLINE const Sphere3V ExpandAbsolute(ScalarV_arg expand) const { return Sphere3V(GetCenter(), GetRadius() + expand); }
	VMATH_INLINE const Sphere3V ExpandRelative(ScalarV_arg expand) const { return Sphere3V(GetCenter(), GetRadius()*expand); }

	VMATH_INLINE bool GetRayIntersection(Vec3V_arg origin,Vec3V_arg dir,Vec3V& nearHit,Vec3V& farHit) const
	{
		// p = o + v*t
		// (p - c).(p - c) = r^2
		// (o + v*t - c).(o + v*t - c) = r^2
		// (o - c + v*t).(o - c + v*t) = r^2
		// (o + v*t).(o + v*t) = r^2
		// oo + vv*t^2 + 2*ov*t = r^2
		// t^2*(vv) + t*(2*ov) + (oo - r^2) == 0
		// t = (-2*ov +/- sqrt(4*ov^2 - 4*vv*(oo - r^2)))/2*vv
		// t = (-ov +/- sqrt(ov^2 - vv*(oo - r^2)))/vv
		const Vec3V o = origin - GetCenter();
		const ScalarV oo = Dot(o,o);
		const ScalarV ov = Dot(o,dir);
		const ScalarV vv = Dot(dir,dir); // could omit this if dir is normalized
		const ScalarV rr = GetRadiusSqr();
		const ScalarV g = ov*ov - vv*(oo - rr);
		if (g >= 0.0f) {
			nearHit = origin + dir*((-ov - Sqrt(g))/vv);
			farHit = origin + dir*((-ov + Sqrt(g))/vv);
			return true;
		} else
			return false;
	}

	Vec3V_out GetRandomPointOnSphereSurface() const;
	Vec3V_out GetRandomPointInSphereInterior() const;

	static const Sphere3V BuildSphereThrough2Points(Vec3V_arg p1,Vec3V_arg p2);
	static const Sphere3V BuildSphereThrough3Points(Vec3V_arg p1,Vec3V_arg p2,Vec3V_arg p3);
	static const Sphere3V BuildSphereThrough4Points(Vec3V_arg p1,Vec3V_arg p2,Vec3V_arg p3,Vec3V_arg p4);

	static const Sphere3V BuildMinimumEnclosingSphere(const Vec3V* points,int count);

	#if 0
	static const Sphere3V BuildMinimumEnclosingSphere_ALT(const Vec3V* points,int count)
	{
		// If we have only a small number of points, can solve with a specialized function.
		switch (count)
		{
		case 0: return Sphere();
		case 1: return Sphere(pts[0],0.f); // Sphere around a single point will be degenerate with r = 0.
		case 2: return OptimalEnclosingSphere(pts[0],pts[1]);
		case 3: return OptimalEnclosingSphere(pts[0],pts[1],pts[2]);
		case 4: return OptimalEnclosingSphere(pts[0],pts[1],pts[2],pts[3]);
		default: break;
		}

		// The set of supporting points for the minimal sphere. Even though the minimal enclosing
		// sphere might have 2, 3 or 4 points in its support (sphere surface), always store here
		// indices to exactly four points.
		int sp[4] ={0,1,2,3};
		// Due to numerical issues, it can happen that the minimal sphere for four points {a,b,c,d} does not
		// accommodate a fifth point e, but replacing any of the points a-d from the support with the point e
		// does not accommodate the all the five points either.
		// Therefore, keep a set of flags for each support point to avoid going in cycles, where the same
		// set of points are again and again added and removed from the support, causing an infinite loop.
		bool expendable[4] ={true,true,true,true};
		// The so-far constructed minimal sphere.
		Sphere s = OptimalEnclosingSphere(pts[sp[0]],pts[sp[1]],pts[sp[2]],pts[sp[3]]);
		float rSq = s.r * s.r + epsilon;
		for(int i = 4; i < numPoints; ++i)
		{
			if(i == sp[0] || i == sp[1] || i == sp[2] || i == sp[3])
				continue; // Take care not to add the same point twice to the support set.
			// If the next point (pts[i]) does not fit inside the currently computed minimal sphere, compute
			// a new minimal sphere that also contains pts[i].
			if(pts[i].DistanceSq(s.pos) > rSq)
			{
				int redundant;
				s = OptimalEnclosingSphere(pts[sp[0]],pts[sp[1]],pts[sp[2]],pts[sp[3]],pts[i],redundant);
				rSq = s.r*s.r + epsilon;
				// A sphere is uniquely defined by four points, so one of the five points passed in above is
				// now redundant, and can be removed from the support set.
				if(redundant != 4 && (sp[redundant] < i || expendable[redundant]))
				{
					sp[redundant] = i; // Replace the old point with the new one.
					expendable[redundant] = false; // This new one cannot be evicted (until we proceed past it in the input list later)
					// Mark all points in the array before this index "expendable", meaning that they can be removed from the support set.
					if(sp[0] < i) expendable[0] = true;
					if(sp[1] < i) expendable[1] = true;
					if(sp[2] < i) expendable[2] = true;
					if(sp[3] < i) expendable[3] = true;

					// Have to start all over and make sure all old points also lie inside this new sphere,
					// since our guess for the minimal enclosing sphere changed.
					i = 0;
				}
			}
		}

		return s;
	}
	#endif

private:
	Vec4V m_center_radius; // xyz=center,w=radius
};

//template <typename T> class Sphere3V_SOA_T // TODO
//{
//public:
//	T m_center;
//	typename T::ComponentType m_radius;
//};
//
//typedef Sphere3V_SOA_T<Vec3V_SOA4> Sphere3V_SOA4;
//#if HAS_VEC8V
//typedef Sphere3V_SOA_T<Vec3V_SOA8> Sphere3V_SOA8;
//#endif // HAS_VEC8V

#endif // _INCLUDE_VMATH_SPHERE_H_

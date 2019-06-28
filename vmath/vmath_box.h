// ========================
// common/vmath/vmath_box.h
// ========================

#ifndef _INCLUDE_VMATH_BOX_H_
#define _INCLUDE_VMATH_BOX_H_

#include "vmath_common.h"
#include "vmath_matrix.h"
#include "vmath_vec4.h"

class Box2V
{
public:
	VMATH_INLINE Box2V() {}
	VMATH_INLINE Box2V(Vec2V_arg min,Vec2V_arg max): m_min(min),m_max(max) {}
	VMATH_INLINE Box2V(float x0,float y0,float x1,float y1): m_min(Vec2V(x0,y0)),m_max(Vec2V(x1,y1)) {}

	VMATH_INLINE static const Box2V Invalid() { return Box2V(Vec2V(FLT_MAX),Vec2V(-FLT_MAX)); }
	VMATH_INLINE bool IsValid() const { return All(m_min<=m_max); }

	VMATH_INLINE Vec2V_out GetMin() const { return m_min; }
	VMATH_INLINE Vec2V_out GetMax() const { return m_max; }
	VMATH_INLINE Vec2V_out GetCenter() const { return (m_max + m_min)*0.5f; }
	VMATH_INLINE Vec2V_out GetExtent() const { return (m_max - m_min)*0.5f; }
	VMATH_INLINE Vec2V_out GetSize() const { return m_max - m_min; }
	VMATH_INLINE const Box2V GetBounds() const { return *this; }

	VMATH_INLINE void Grow(Vec2V_arg point) { m_min = Min(point,m_min); m_max = Max(point,m_max); }
	VMATH_INLINE void Grow(const Box2V& box) { m_min = Min(box.m_min,m_min); m_max = Max(box.m_max,m_max); }
	template <typename T> VMATH_INLINE void Grow(const T& obj) { Grow(obj.GetBounds()); }

	VMATH_INLINE ScalarV_out GetArea() const { const Vec2V size = m_max - m_min; return size.x()*size.y(); }

	VMATH_INLINE void GetCorners(Vec2V corners[4]) const
	{
		corners[0] = Select00(m_min,m_max);
		corners[1] = Select01(m_min,m_max);
		corners[2] = Select10(m_min,m_max);
		corners[3] = Select11(m_min,m_max);
	}

	VMATH_INLINE ScalarV_out GetDistanceToPoint(Vec2V_arg point) const { return Mag(Max(Vec2V(V_ZERO),Abs(point - GetCenter()) - GetExtent())); }

	VMATH_INLINE static const Box2V Load(const void* src)
	{
		const Vec2V bmin = Vec2V::Load(reinterpret_cast<const float*>(src) + 0);
		const Vec2V bmax = Vec2V::Load(reinterpret_cast<const float*>(src) + 2);
		return Box2V(bmin,bmax);
	}

	VMATH_INLINE void Store(void* dst) const
	{
		m_min.Store(reinterpret_cast<float*>(dst) + 0);
		m_max.Store(reinterpret_cast<float*>(dst) + 2);
	}

private:
	Vec2V m_min;
	Vec2V m_max;
};

#define BOX_RAY_INTERSECT_OLD_CODE (1) // this runs faster in my current tests, so keep it
#define BOX_RAY_INTERSECT_ROBUST //*1.00000024f // see Robust BVH Ray Traversal by Thiago Ize

template <typename T> class Box3V_SOA_T;

class Box3V
{
public:
	VMATH_INLINE Box3V() {}
	VMATH_INLINE Box3V(Vec3V_arg min,Vec3V_arg max): m_min(min),m_max(max) {}

	VMATH_INLINE static const Box3V Invalid() { return Box3V(Vec3V(FLT_MAX),Vec3V(-FLT_MAX)); }
	VMATH_INLINE bool IsValid() const { return All(m_min<=m_max); }

	VMATH_INLINE Vec3V_out GetMin() const { return m_min; }
	VMATH_INLINE Vec3V_out GetMax() const { return m_max; }
	VMATH_INLINE Vec3V_out GetCenter() const { return (m_max + m_min)*0.5f; }
	VMATH_INLINE Vec3V_out GetExtent() const { return (m_max - m_min)*0.5f; }
	VMATH_INLINE Vec3V_out GetSize() const { return m_max - m_min; }
	VMATH_INLINE const Box3V GetBounds() const { return *this; }

	VMATH_INLINE void Grow(Vec3V_arg point) { m_min = Min(point,m_min); m_max = Max(point,m_max); }
	VMATH_INLINE void Grow(const Box3V& box) { m_min = Min(box.m_min,m_min); m_max = Max(box.m_max,m_max); }
	template <typename T> VMATH_INLINE void Grow(const T& obj) { Grow(obj.GetBounds()); }

	VMATH_INLINE const Box3V ExpandAbsolute(Vec3V_arg expand) const
	{
		const Vec3V center = GetCenter();
		const Vec3V extent = GetExtent() + expand;
		return Box3V(center - extent,center + extent);
	}

	VMATH_INLINE const Box3V ExpandRelative(Vec3V_arg expand) const
	{
		const Vec3V center = GetCenter();
		const Vec3V extent = GetExtent()*expand;
		return Box3V(center - extent,center + extent);
	}

	VMATH_INLINE ScalarV_out GetVolume() const
	{
		const Vec3V size = m_max - m_min;
		return size.x()*size.y()*size.z();
	}

	VMATH_INLINE void GetCorners(Vec3V corners[8]) const
	{
		corners[0] = Select000(m_min,m_max);
		corners[1] = Select001(m_min,m_max);
		corners[2] = Select010(m_min,m_max);
		corners[3] = Select011(m_min,m_max);
		corners[4] = Select100(m_min,m_max);
		corners[5] = Select101(m_min,m_max);
		corners[6] = Select110(m_min,m_max);
		corners[7] = Select111(m_min,m_max);
	}

	VMATH_INLINE ScalarV_out GetDistanceToPoint(Vec3V_arg point) const { return Mag(Max(Vec3V(V_ZERO),Abs(point - GetCenter()) - GetExtent())); }

	template <typename OriginType,typename DirType> VMATH_INLINE uint32 IntersectsRay(const OriginType& origin,const DirType& invdir,typename DirType::ComponentType::ArgType t) const
	{
		// origin + t*v = bmin,bmax
		// solve for t
	#if BOX_RAY_INTERSECT_OLD_CODE
		const DirType a = (m_min - origin)*invdir;
		const DirType b = (m_max - origin)*invdir;
		const DirType tmin = Min(a,b);
		const DirType tmax = Max(a,b);
	#else
		const DirType tmin = (Select(m_min,m_max,invdir) - origin)*invdir;
		const DirType tmax = (Select(m_max,m_min,invdir) - origin)*invdir;
	#endif
		const typename DirType::ComponentType t0 = Max(MaxElement(tmin),typename DirType::ComponentType(V_ZERO));
		const typename DirType::ComponentType t1 = Min(MinElement(tmax),t);
		return GetBoolMask(t0 <= t1 BOX_RAY_INTERSECT_ROBUST);
	}

	template <typename OriginType,typename DirType> VMATH_INLINE uint32 IntersectsRay_return_t(const OriginType& origin,const DirType& invdir,typename DirType::ComponentType& t) const
	{
		const DirType a = (m_min - origin)*invdir;
		const DirType b = (m_max - origin)*invdir;
		const typename DirType::ComponentType t0 = Max(MaxElement(Min(a,b)),DirType::ComponentType(V_ZERO));
		const typename DirType::ComponentType t1 = Min(MinElement(Max(a,b)),t);
		t = t0;
		return GetBoolMask(t0 <= t1 BOX_RAY_INTERSECT_ROBUST);
	}

	template <typename OriginType,typename DirType> VMATH_INLINE uint32 IntersectsRay_return_t0_t1(const OriginType& origin,const DirType& invdir,typename DirType::ComponentType& t0,typename DirType::ComponentType& t1) const
	{
		const DirType a = (m_min - origin)*invdir;
		const DirType b = (m_max - origin)*invdir;
		t0 = Max(MaxElement(Min(a,b)),t0);
		t1 = Min(MinElement(Max(a,b)),t1);
		return GetBoolMask(t0 <= t1 BOX_RAY_INTERSECT_ROBUST);
	}

	template <typename BoxType> VMATH_INLINE const BoxType BroadcastSOA() const
	{
		typedef typename BoxType::T::ComponentType ComponentType;
		const typename BoxType::T bmin(
			ComponentType::LoadScalar(&m_min.xf_constref()),
			ComponentType::LoadScalar(&m_min.yf_constref()),
			ComponentType::LoadScalar(&m_min.zf_constref()));
		const typename BoxType::T bmax(
			ComponentType::LoadScalar(&m_max.xf_constref()),
			ComponentType::LoadScalar(&m_max.yf_constref()),
			ComponentType::LoadScalar(&m_max.zf_constref()));
		return BoxType(bmin,bmax);
	}

private:
	Vec3V m_min;
	Vec3V m_max;
};

template <> VMATH_INLINE const Box3V Box3V::BroadcastSOA<Box3V>() const
{
	return *this;
}

template <typename T_> class Box3V_SOA_T
{
public:
	typedef T_ T;
	enum { NumBoxes = T::NumVectors };

	VMATH_INLINE Box3V_SOA_T() {}
	VMATH_INLINE Box3V_SOA_T(typename T::ArgType bmin,typename T::ArgType bmax): m_min(bmin),m_max(bmax) {}
	VMATH_INLINE Box3V_SOA_T(const Box3V boxes[NumBoxes])
	{
		Vec3V bmin[NumBoxes];
		Vec3V bmax[NumBoxes];
		for (unsigned i = 0; i < NumBoxes; i++) {
			bmin[i] = boxes[i].GetMin();
			bmax[i] = boxes[i].GetMax();
		}
		m_min = T(bmin);
		m_max = T(bmax);
	}

	VMATH_INLINE typename T::OutType GetMin() const { return m_min; }
	VMATH_INLINE typename T::OutType GetMax() const { return m_max; }
	VMATH_INLINE typename T::OutType GetCenter() const { return (m_max + m_min)*0.5f; }
	VMATH_INLINE typename T::OutType GetExtent() const { return (m_max - m_min)*0.5f; }
	VMATH_INLINE typename T::OutType GetSize() const { return m_max - m_min; }

	VMATH_INLINE const Box3V GetIndexed(unsigned index) const
	{
		DEBUG_ASSERT(index < NumBoxes);
		return Box3V(m_min.GetVector(index),m_max.GetVector(index));
	}

	template <typename BoxType> VMATH_INLINE const BoxType GetIndexed_BroadcastSOA(unsigned index) const // single Box3V expanded for SOA - can be intersected against multiple rays
	{
		DEBUG_ASSERT(index < NumBoxes);
		typedef typename BoxType::T::ComponentType ComponentType;
		const typename BoxType::T bmin(
			ComponentType::LoadScalar(&m_min.x_constref() + index),
			ComponentType::LoadScalar(&m_min.y_constref() + index),
			ComponentType::LoadScalar(&m_min.z_constref() + index));
		const typename BoxType::T bmax(
			ComponentType::LoadScalar(&m_max.x_constref() + index),
			ComponentType::LoadScalar(&m_max.y_constref() + index),
			ComponentType::LoadScalar(&m_max.z_constref() + index));
		return BoxType(bmin,bmax);
	}

	// note that IntersectsRay can be called with DirType as an SOA vector, if box has been constructed from a single Box3V expanded for SOA
	template <typename OriginType,typename DirType> VMATH_INLINE uint32 IntersectsRay(const OriginType& origin,const DirType& invdir,typename T::ComponentType::ArgType t) const
	{
		const T a = (m_min - origin)*invdir;
		const T b = (m_max - origin)*invdir;
		const typename T::ComponentType t0 = Max(MaxElement(Min(a,b)),typename T::ComponentType(V_ZERO));
		const typename T::ComponentType t1 = Min(MinElement(Max(a,b)),t);
		return GetBoolMask(t0 <= t1 BOX_RAY_INTERSECT_ROBUST);
	}

private:
	T m_min;
	T m_max;
};

typedef Box3V_SOA_T<class Vec3V_SOA4> Box3V_SOA4;
#if HAS_VEC8V
typedef Box3V_SOA_T<class Vec3V_SOA8> Box3V_SOA8;
#if HAS_VEC16V
typedef Box3V_SOA_T<class Vec3V_SOA16> Box3V_SOA16;
#endif // HAS_VEC16V
#endif // HAS_VEC8V

class YOrientedBox3V
{
public:
	VMATH_INLINE YOrientedBox3V() {}

	VMATH_INLINE const Box2V GetLocalBoxXZ() const { return Box2V(Vec2V(_vmath_permute_ps<0,2,0,0>(m_minxyz_cosangle)),Vec2V(_vmath_permute_ps<0,2,0,0>(m_maxxyz_sinangle))); }
	VMATH_INLINE const Box3V GetLocalBox() const { return Box3V(m_minxyz_cosangle.xyz(),m_maxxyz_sinangle.xyz()); }

	VMATH_INLINE Vec3V_out GetLocalMin() const { return m_minxyz_cosangle.xyz(); }
	VMATH_INLINE Vec3V_out GetLocalMax() const { return m_maxxyz_sinangle.xyz(); }
	VMATH_INLINE Vec3V_out GetLocalCenter() const { return (m_maxxyz_sinangle + m_minxyz_cosangle).xyz()*0.5f; }
	VMATH_INLINE Vec3V_out GetLocalExtent() const { return (m_maxxyz_sinangle - m_minxyz_cosangle).xyz()*0.5f; }

	VMATH_INLINE ScalarV_out GetCosAngle() const { return m_minxyz_cosangle.w(); }
	VMATH_INLINE ScalarV_out GetSinAngle() const { return m_maxxyz_sinangle.w(); }

	VMATH_INLINE ScalarV_out GetAngle() const { return ScalarV(atan2f(GetCosAngle().f(),GetSinAngle().f())); }

private:
	Vec4V m_minxyz_cosangle; // xyz = min, w = cos(angle)
	Vec4V m_maxxyz_sinangle; // xyz = max, w = sin(angle)
};

class OrientedBox3V
{
public:
	OrientedBox3V() {}

	//VMATH_INLINE Vec3V_out GetLocalMin() const { return GetLocalCenter() - GetLocalExtent(); }
	//VMATH_INLINE Vec3V_out GetLocalMax() const { return GetLocalCenter() + GetLocalExtent(); }
	VMATH_INLINE Vec3V_out GetLocalCenter() const { return m_matrixd; }
	VMATH_INLINE Vec3V_out GetLocalExtent() const { return Vec3V(m_matrixa_extentx.w(),m_matrixb_extenty.w(),m_matrixc_extentz.w()); }

	VMATH_INLINE Mat34V_out GetMat34V() const { return m_matrix; }

	VMATH_INLINE Vec3V_out LocalToWorld(Vec3V_arg point) const { return point; } // TODO!
	VMATH_INLINE Vec3V_out WorldToLocal(Vec3V_arg point) const { return point; } // TODO!

private:
	union {
		struct { Mat34V m_matrix; };
		struct {
			Vec4V m_matrixa_extentx; // xyz = matrix column a, w = extent x
			Vec4V m_matrixb_extenty; // xyz = matrix column b, w = extent y
			Vec4V m_matrixc_extentz; // xyz = matrix column c, w = extent z
			Vec3V m_matrixd;
		};
	};
};

#endif // _INCLUDE_VMATH_BOX_H_

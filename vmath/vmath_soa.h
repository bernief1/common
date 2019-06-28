// ========================
// common/vmath/vmath_soa.h
// ========================

#ifndef _INCLUDE_VMATH_SOA_H_
#define _INCLUDE_VMATH_SOA_H_

#include "vmath_common.h"
#include "vmath_vec16.h"
#include "vmath_matrix.h"

template <unsigned> class SOA_T {};

class Vec2V_SOA4; // TODO
class Vec4V_SOA4; // TODO

class Vec3V_SOA4 // four Vec3V's stored in transposed form
{
public:
	typedef Vec3V_SOA4 T;
	typedef const T& ArgType;
	typedef const T OutType;
	typedef Vec3V VectorType;
	typedef typename VectorType::ArgType VectorType_arg;
	typedef typename VectorType::OutType VectorType_out;
	typedef Vec4V ComponentType;
	typedef typename ComponentType::ArgType ComponentType_arg;
	typedef typename ComponentType::OutType ComponentType_out;
	enum { NumVectors = ComponentType::NumElements };
	typedef typename VectorType::ElementType ElementType;

	VMATH_INLINE Vec3V_SOA4() {}
	VMATH_INLINE explicit Vec3V_SOA4(ElementType f) { m_x = m_y = m_z = ScalarV(f); }
	VMATH_INLINE Vec3V_SOA4(ComponentType_arg xyz): m_x(xyz),m_y(xyz),m_z(xyz) {} // copies xyz into each of the four Vec3V's
	VMATH_INLINE Vec3V_SOA4(VectorType_arg v): m_x(v.x()),m_y(v.y()),m_z(v.z()) {} // treats 'v' as three ScalarV's ..
	VMATH_INLINE Vec3V_SOA4(ComponentType_arg x,ComponentType_arg y,ComponentType_arg z): m_x(x),m_y(y),m_z(z) {}
	VMATH_INLINE Vec3V_SOA4(VectorType_arg v0,VectorType_arg v1,VectorType_arg v2,VectorType_arg v3) { Mat34VTranspose(m_x,m_y,m_z,v0,v1,v2,v3); }
	VMATH_INLINE Vec3V_SOA4(const VectorType v[NumVectors]) { Mat34VTranspose(m_x,m_y,m_z,v[0],v[1],v[2],v[3]); }

	VMATH_INLINE ComponentType_out x() const { return m_x; }
	VMATH_INLINE ComponentType_out y() const { return m_y; }
	VMATH_INLINE ComponentType_out z() const { return m_z; }

	VMATH_INLINE ComponentType& x_ref() { return m_x; }
	VMATH_INLINE ComponentType& y_ref() { return m_y; }
	VMATH_INLINE ComponentType& z_ref() { return m_z; }
	VMATH_INLINE const ComponentType& x_constref() const { return m_x; }
	VMATH_INLINE const ComponentType& y_constref() const { return m_y; }
	VMATH_INLINE const ComponentType& z_constref() const { return m_z; }

	VMATH_INLINE void GetVectors(VectorType v[NumVectors]) const { Mat43VTranspose(v[0],v[1],v[2],v[3],m_x,m_y,m_z); }
	VMATH_INLINE VectorType_out GetVector(unsigned index) const { return VectorType(m_x[index],m_y[index],m_z[index]); }

	VMATH_INLINE friend OutType operator +(ArgType a) { return a; } // no-op
	VMATH_INLINE friend OutType operator -(ArgType a) { return T(-a.x(),-a.y(),-a.z()); }
	VMATH_INLINE friend OutType operator +(ArgType a,ArgType b) { return T(a.x() + b.x(),a.y() + b.y(),a.z() + b.z()); }
	VMATH_INLINE friend OutType operator -(ArgType a,ArgType b) { return T(a.x() - b.x(),a.y() - b.y(),a.z() - b.z()); }
	VMATH_INLINE friend OutType operator *(ArgType a,ArgType b) { return T(a.x() * b.x(),a.y() * b.y(),a.z() * b.z()); }
	VMATH_INLINE friend OutType operator /(ArgType a,ArgType b) { return T(a.x() / b.x(),a.y() / b.y(),a.z() / b.z()); }
	VMATH_INLINE friend OutType operator *(ArgType a,ComponentType_arg b) { return T(a.x() * b,a.y() * b,a.z() * b); }
	VMATH_INLINE friend OutType operator /(ArgType a,ComponentType_arg b) { return T(a.x() / b,a.y() / b,a.z() / b); }
	VMATH_INLINE friend OutType operator *(ArgType a,ElementType b) { return T(a.x() * b,a.y() * b,a.z() * b); }
	VMATH_INLINE friend OutType operator /(ArgType a,ElementType b) { return T(a.x() / b,a.y() / b,a.z() / b); }
	VMATH_INLINE friend OutType operator *(ElementType a,ArgType b) { return T(a * b.x(),a * b.y(),a * b.z()); }
	VMATH_INLINE friend OutType operator /(ElementType a,ArgType b) { return T(a / b.x(),a / b.y(),a / b.z()); }
	
	VMATH_INLINE T& operator +=(ArgType b) { return(*this = *this + b); }
	VMATH_INLINE T& operator -=(ArgType b) { return(*this = *this - b); }
	VMATH_INLINE T& operator *=(ArgType b) { return(*this = *this * b); }
	VMATH_INLINE T& operator /=(ArgType b) { return(*this = *this / b); }
	VMATH_INLINE T& operator *=(ComponentType_arg b) { return(*this = *this * b); }
	VMATH_INLINE T& operator /=(ComponentType_arg b) { return(*this = *this / b); }
	VMATH_INLINE T& operator *=(ElementType b) { return(*this = *this * b); }
	VMATH_INLINE T& operator /=(ElementType b) { return(*this = *this / b); }
	
#define PER_COMPONENT_FUNC1(func,a) T(func(a.x()),func(a.y()),func(a.z()))
#define PER_COMPONENT_FUNC2(func,a,b) T(func(a.x(),b.x()),func(a.y(),b.y()),func(a.z(),b.z()))
#define PER_COMPONENT_FUNC3(func,a,b,c) T(func(a.x(),b.x(),c.x()),func(a.y(),b.y(),c.y()),func(a.z(),b.z(),c.z()))
#define PER_COMPONENT_FUNC4(func,a,b,c,d) T(func(a.x(),b.x(),c.x(),d.x()),func(a.y(),b.y(),c.y(),d.y()),func(a.z(),b.z(),c.z(),d.z()))
	VMATH_INLINE friend OutType Min(ArgType a,ArgType b) { return PER_COMPONENT_FUNC2(Min,a,b); }
	VMATH_INLINE friend OutType Max(ArgType a,ArgType b) { return PER_COMPONENT_FUNC2(Max,a,b); }
	VMATH_INLINE friend OutType Min(ArgType a,ArgType b,ArgType c) { return PER_COMPONENT_FUNC3(Min,a,b,c); }
	VMATH_INLINE friend OutType Max(ArgType a,ArgType b,ArgType c) { return PER_COMPONENT_FUNC3(Max,a,b,c); }
	VMATH_INLINE friend OutType Min(ArgType a,ArgType b,ArgType c,ArgType d) { return PER_COMPONENT_FUNC4(Min,a,b,c,d); }
	VMATH_INLINE friend OutType Max(ArgType a,ArgType b,ArgType c,ArgType d) { return PER_COMPONENT_FUNC4(Max,a,b,c,d); }
	VMATH_INLINE friend OutType Abs(ArgType a) { return PER_COMPONENT_FUNC1(Abs,a); }
	VMATH_INLINE friend OutType Clamp(ArgType a,ArgType minVal,ArgType maxVal) { return PER_COMPONENT_FUNC3(Clamp,a,minVal,maxVal); }
	VMATH_INLINE friend OutType Saturate(ArgType a) { return PER_COMPONENT_FUNC1(Saturate,a); }
	VMATH_INLINE friend OutType Floor(ArgType a) { return PER_COMPONENT_FUNC1(Floor,a); }
	VMATH_INLINE friend OutType Ceiling(ArgType a) { return PER_COMPONENT_FUNC1(Ceiling,a); }
	VMATH_INLINE friend OutType Truncate(ArgType a) { return PER_COMPONENT_FUNC1(Truncate,a); }
	VMATH_INLINE friend OutType Round(ArgType a) { return PER_COMPONENT_FUNC1(Round,a); }
	VMATH_INLINE friend OutType Frac(ArgType a) { return PER_COMPONENT_FUNC1(Frac,a); }
	VMATH_INLINE friend OutType Sqrt(ArgType a) { return PER_COMPONENT_FUNC1(Sqrt,a); }
	VMATH_INLINE friend OutType Sqr(ArgType a) { return PER_COMPONENT_FUNC1(Sqr,a); }
	VMATH_INLINE friend OutType Recip(ArgType a) { return PER_COMPONENT_FUNC1(Recip,a); }
	VMATH_INLINE friend OutType RecipSqrt(ArgType a) { return PER_COMPONENT_FUNC1(RecipSqrt,a); }
#undef PER_COMPONENT_FUNC1
#undef PER_COMPONENT_FUNC2
#undef PER_COMPONENT_FUNC3
#undef PER_COMPONENT_FUNC4

	VMATH_INLINE friend ComponentType_out MinElement(ArgType a) { return Min(a.x(),a.y(),a.z()); }
	VMATH_INLINE friend ComponentType_out MaxElement(ArgType a) { return Max(a.x(),a.y(),a.z()); }

	VMATH_INLINE friend ComponentType_out Dot(ArgType a,ArgType b) { return MultiplyAdd(a.x(),b.x(),a.y(),b.y(),a.z(),b.z()); }
	VMATH_INLINE friend ComponentType_out MagSqr(ArgType a) { return Dot(a,a); }
	VMATH_INLINE friend ComponentType_out Mag(ArgType a) { return Sqrt(MagSqr(a)); }

	VMATH_INLINE friend OutType Cross(ArgType a,ArgType b)
	{
		return T(
			MultiplySub(a.y(),b.z(),b.y(),a.z()),  // a.y*b.z - b.y*a.z
			MultiplySub(a.z(),b.x(),b.z(),a.x()),  // a.z*b.x - b.z*a.x
			MultiplySub(a.x(),b.y(),b.x(),a.y())); // a.x*b.y - b.x*a.y
	}

	VMATH_INLINE friend OutType Normalize(ArgType a) { return a*RecipSqrt(MagSqr(a)); }

	VMATH_INLINE friend OutType Select(ArgType a,ArgType b,ArgType sel)
	{
		return T(
			Select(a.x(),b.x(),ComponentType::BoolV(sel.x())),
			Select(a.y(),b.y(),ComponentType::BoolV(sel.y())),
			Select(a.z(),b.z(),ComponentType::BoolV(sel.z())));
	}

	ComponentType m_x,m_y,m_z;
};

typedef typename Vec3V_SOA4::ArgType Vec3V_SOA4_arg;
typedef typename Vec3V_SOA4::OutType Vec3V_SOA4_out;

template <> class SOA_T<4>
{
public:
	typedef Vec3V_SOA4 Vec3V_SOAType;
};

VMATH_INLINE void PrintV(const char* name,Vec3V_SOA4_arg v)
{
	printf("%s=\n\t{%f,%f,%f,%f}\n\t{%f,%f,%f,%f}\n\t{%f,%f,%f,%f}\n",name,VEC4V_ARGS(v.x()),VEC4V_ARGS(v.y()),VEC4V_ARGS(v.z()));
}

#if HAS_VEC8V
class Vec3V_SOA8 // eight Vec3V's stored in transposed form
{
public:
	typedef Vec3V_SOA8 T;
	typedef const T& ArgType;
	typedef const T OutType;
	typedef Vec3V VectorType;
	typedef typename VectorType::ArgType VectorType_arg;
	typedef typename VectorType::OutType VectorType_out;
	typedef Vec8V ComponentType;
	typedef typename ComponentType::ArgType ComponentType_arg;
	typedef typename ComponentType::OutType ComponentType_out;
	enum { NumVectors = ComponentType::NumElements };
	typedef typename VectorType::ElementType ElementType;

	VMATH_INLINE Vec3V_SOA8() {}
	VMATH_INLINE explicit Vec3V_SOA8(ElementType f) { m_x = m_y = m_z = ScalarV(f); }
	VMATH_INLINE Vec3V_SOA8(ComponentType_arg v): m_x(v),m_y(v),m_z(v) {} // copies xyz into each of the eight VectorType's
	VMATH_INLINE Vec3V_SOA8(VectorType_arg v): m_x(v.x()),m_y(v.y()),m_z(v.z()) {} // treats 'v' as three ScalarV's ..
	VMATH_INLINE Vec3V_SOA8(Vec3V_SOA4_arg v0,Vec3V_SOA4_arg v1):
		m_x(v0.x(),v1.x()),
		m_y(v0.y(),v1.y()),
		m_z(v0.z(),v1.z()) {}
	VMATH_INLINE Vec3V_SOA8(ComponentType_arg x,ComponentType_arg y,ComponentType_arg z): m_x(x),m_y(y),m_z(z) {}
	VMATH_INLINE Vec3V_SOA8(
		VectorType_arg v0,VectorType_arg v1,VectorType_arg v2,VectorType_arg v3,
		VectorType_arg v4,VectorType_arg v5,VectorType_arg v6,VectorType_arg v7)
	{
		Vec4V x0,y0,z0; Mat34VTranspose(x0,y0,z0,v0,v1,v2,v3);
		Vec4V x1,y1,z1; Mat34VTranspose(x1,y1,z1,v4,v5,v6,v7);
		m_x = ComponentType(x0,x1);
		m_y = ComponentType(y0,y1);
		m_z = ComponentType(z0,z1);
	}
	VMATH_INLINE Vec3V_SOA8(const VectorType v[NumVectors])
	{
		Vec4V x0,y0,z0; Mat34VTranspose(x0,y0,z0,v[0],v[1],v[2],v[3]);
		Vec4V x1,y1,z1; Mat34VTranspose(x1,y1,z1,v[4],v[5],v[6],v[7]);
		m_x = ComponentType(x0,x1);
		m_y = ComponentType(y0,y1);
		m_z = ComponentType(z0,z1);
	}

	VMATH_INLINE static OutType Zero() { return ComponentType(V_ZERO); }
	VMATH_INLINE static OutType One() { return ComponentType(V_ONE); }

	VMATH_INLINE ComponentType_out x() const { return m_x; }
	VMATH_INLINE ComponentType_out y() const { return m_y; }
	VMATH_INLINE ComponentType_out z() const { return m_z; }

	VMATH_INLINE ComponentType& x_ref() { return m_x; }
	VMATH_INLINE ComponentType& y_ref() { return m_y; }
	VMATH_INLINE ComponentType& z_ref() { return m_z; }
	VMATH_INLINE const ComponentType& x_constref() const { return m_x; }
	VMATH_INLINE const ComponentType& y_constref() const { return m_y; }
	VMATH_INLINE const ComponentType& z_constref() const { return m_z; }

	VMATH_INLINE typename Vec3V_SOA4_out v0() const { return Vec3V_SOA4(x().v0(),y().v0(),z().v0()); }
	VMATH_INLINE typename Vec3V_SOA4_out v1() const { return Vec3V_SOA4(x().v1(),y().v1(),z().v1()); }

	VMATH_INLINE void GetVectors(VectorType v[NumVectors]) const
	{
		Mat43VTranspose(v[0],v[1],v[2],v[3],m_x.v0(),m_y.v0(),m_z.v0());
		Mat43VTranspose(v[4],v[5],v[6],v[7],m_x.v1(),m_y.v1(),m_z.v1());
	}
	VMATH_INLINE VectorType_out GetVector(unsigned index) const { return VectorType(m_x[index],m_y[index],m_z[index]); }

	VMATH_INLINE friend OutType operator +(ArgType a) { return a; } // no-op
	VMATH_INLINE friend OutType operator -(ArgType a) { return T(-a.x(),-a.y(),-a.z()); }
	VMATH_INLINE friend OutType operator +(ArgType a,ArgType b) { return T(a.x() + b.x(),a.y() + b.y(),a.z() + b.z()); }
	VMATH_INLINE friend OutType operator -(ArgType a,ArgType b) { return T(a.x() - b.x(),a.y() - b.y(),a.z() - b.z()); }
	VMATH_INLINE friend OutType operator *(ArgType a,ArgType b) { return T(a.x() * b.x(),a.y() * b.y(),a.z() * b.z()); }
	VMATH_INLINE friend OutType operator /(ArgType a,ArgType b) { return T(a.x() / b.x(),a.y() / b.y(),a.z() / b.z()); }
	VMATH_INLINE friend OutType operator *(ArgType a,ComponentType_arg b) { return T(a.x() * b,a.y() * b,a.z() * b); }
	VMATH_INLINE friend OutType operator /(ArgType a,ComponentType_arg b) { return T(a.x() / b,a.y() / b,a.z() / b); }
	VMATH_INLINE friend OutType operator *(ArgType a,ElementType b) { return T(a.x() * b,a.y() * b,a.z() * b); }
	VMATH_INLINE friend OutType operator /(ArgType a,ElementType b) { return T(a.x() / b,a.y() / b,a.z() / b); }
	VMATH_INLINE friend OutType operator *(ElementType a,ArgType b) { return T(a * b.x(),a * b.y(),a * b.z()); }
	VMATH_INLINE friend OutType operator /(ElementType a,ArgType b) { return T(a / b.x(),a / b.y(),a / b.z()); }
	
	VMATH_INLINE T& operator +=(ArgType b) { return(*this = *this + b); }
	VMATH_INLINE T& operator -=(ArgType b) { return(*this = *this - b); }
	VMATH_INLINE T& operator *=(ArgType b) { return(*this = *this * b); }
	VMATH_INLINE T& operator /=(ArgType b) { return(*this = *this / b); }
	VMATH_INLINE T& operator *=(ComponentType_arg b) { return(*this = *this * b); }
	VMATH_INLINE T& operator /=(ComponentType_arg b) { return(*this = *this / b); }
	VMATH_INLINE T& operator *=(ElementType b) { return(*this = *this * b); }
	VMATH_INLINE T& operator /=(ElementType b) { return(*this = *this / b); }

#define PER_COMPONENT_FUNC1(func,a) T(func(a.x()),func(a.y()),func(a.z()))
#define PER_COMPONENT_FUNC2(func,a,b) T(func(a.x(),b.x()),func(a.y(),b.y()),func(a.z(),b.z()))
#define PER_COMPONENT_FUNC3(func,a,b,c) T(func(a.x(),b.x(),c.x()),func(a.y(),b.y(),c.y()),func(a.z(),b.z(),c.z()))
#define PER_COMPONENT_FUNC4(func,a,b,c,d) T(func(a.x(),b.x(),c.x(),d.x()),func(a.y(),b.y(),c.y(),d.y()),func(a.z(),b.z(),c.z(),d.z()))
	VMATH_INLINE friend OutType Min(ArgType a,ArgType b) { return PER_COMPONENT_FUNC2(Min,a,b); }
	VMATH_INLINE friend OutType Max(ArgType a,ArgType b) { return PER_COMPONENT_FUNC2(Max,a,b); }
	VMATH_INLINE friend OutType Min(ArgType a,ArgType b,ArgType c) { return PER_COMPONENT_FUNC3(Min,a,b,c); }
	VMATH_INLINE friend OutType Max(ArgType a,ArgType b,ArgType c) { return PER_COMPONENT_FUNC3(Max,a,b,c); }
	VMATH_INLINE friend OutType Min(ArgType a,ArgType b,ArgType c,ArgType d) { return PER_COMPONENT_FUNC4(Min,a,b,c,d); }
	VMATH_INLINE friend OutType Max(ArgType a,ArgType b,ArgType c,ArgType d) { return PER_COMPONENT_FUNC4(Max,a,b,c,d); }
	VMATH_INLINE friend OutType Abs(ArgType a) { return PER_COMPONENT_FUNC1(Abs,a); }
	VMATH_INLINE friend OutType Clamp(ArgType a,ArgType minVal,ArgType maxVal) { return PER_COMPONENT_FUNC3(Clamp,a,minVal,maxVal); }
	VMATH_INLINE friend OutType Saturate(ArgType a) { return PER_COMPONENT_FUNC1(Saturate,a); }
	VMATH_INLINE friend OutType Floor(ArgType a) { return PER_COMPONENT_FUNC1(Floor,a); }
	VMATH_INLINE friend OutType Ceiling(ArgType a) { return PER_COMPONENT_FUNC1(Ceiling,a); }
	VMATH_INLINE friend OutType Truncate(ArgType a) { return PER_COMPONENT_FUNC1(Truncate,a); }
	VMATH_INLINE friend OutType Round(ArgType a) { return PER_COMPONENT_FUNC1(Round,a); }
	VMATH_INLINE friend OutType Frac(ArgType a) { return PER_COMPONENT_FUNC1(Frac,a); }
	VMATH_INLINE friend OutType Sqrt(ArgType a) { return PER_COMPONENT_FUNC1(Sqrt,a); }
	VMATH_INLINE friend OutType Sqr(ArgType a) { return PER_COMPONENT_FUNC1(Sqr,a); }
	VMATH_INLINE friend OutType Recip(ArgType a) { return PER_COMPONENT_FUNC1(Recip,a); }
	VMATH_INLINE friend OutType RecipSqrt(ArgType a) { return PER_COMPONENT_FUNC1(RecipSqrt,a); }
#undef PER_COMPONENT_FUNC1
#undef PER_COMPONENT_FUNC2
#undef PER_COMPONENT_FUNC3
#undef PER_COMPONENT_FUNC4

	VMATH_INLINE friend ComponentType_out MinElement(ArgType a) { return Min(a.x(),a.y(),a.z()); }
	VMATH_INLINE friend ComponentType_out MaxElement(ArgType a) { return Max(a.x(),a.y(),a.z()); }

	VMATH_INLINE friend ComponentType_out Dot(ArgType a,ArgType b) { return MultiplyAdd(a.x(),b.x(),a.y(),b.y(),a.z(),b.z()); }
	VMATH_INLINE friend ComponentType_out MagSqr(ArgType a) { return Dot(a,a); }
	VMATH_INLINE friend ComponentType_out Mag(ArgType a) { return Sqrt(MagSqr(a)); }

	VMATH_INLINE friend OutType Cross(ArgType a,ArgType b)
	{
		return T(
			MultiplySub(a.y(),b.z(),b.y(),a.z()),  // a.y*b.z - b.y*a.z
			MultiplySub(a.z(),b.x(),b.z(),a.x()),  // a.z*b.x - b.z*a.x
			MultiplySub(a.x(),b.y(),b.x(),a.y())); // a.x*b.y - b.x*a.y
	}

	VMATH_INLINE friend OutType Normalize(ArgType a) { return a*RecipSqrt(MagSqr(a)); }

	VMATH_INLINE friend OutType Select(ArgType a,ArgType b,ArgType sel)
	{
		return T(
			Select(a.x(),b.x(),ComponentType::BoolV(sel.x())),
			Select(a.y(),b.y(),ComponentType::BoolV(sel.y())),
			Select(a.z(),b.z(),ComponentType::BoolV(sel.z())));
	}

	ComponentType m_x,m_y,m_z;
};

typedef typename Vec3V_SOA8::ArgType Vec3V_SOA8_arg;
typedef typename Vec3V_SOA8::OutType Vec3V_SOA8_out;

template <> class SOA_T<8>
{
public:
	typedef Vec3V_SOA8 Vec3V_SOAType;
};

VMATH_INLINE void PrintV(const char* name,Vec3V_SOA8_arg v)
{
	printf("%s=\n\t{%f,%f,%f,%f, %f,%f,%f,%f}\n\t{%f,%f,%f,%f, %f,%f,%f,%f}\n\t{%f,%f,%f,%f, %f,%f,%f,%f}\n",name,VEC8V_ARGS(v.x()),VEC8V_ARGS(v.y()),VEC8V_ARGS(v.z()));
}

#if HAS_VEC16V
class Vec3V_SOA16 // sixteen Vec3V's stored in transposed form
{
public:
	typedef Vec3V_SOA16 T;
	typedef const T& ArgType;
	typedef const T OutType;
	typedef Vec3V VectorType;
	typedef typename VectorType::ArgType VectorType_arg;
	typedef typename VectorType::OutType VectorType_out;
	typedef Vec16V ComponentType;
	typedef typename ComponentType::ArgType ComponentType_arg;
	typedef typename ComponentType::OutType ComponentType_out;
	enum { NumVectors = ComponentType::NumElements };
	typedef typename VectorType::ElementType ElementType;

	VMATH_INLINE Vec3V_SOA16() {}
	VMATH_INLINE explicit Vec3V_SOA16(ElementType f) { m_x = m_y = m_z = ScalarV(f); }
	VMATH_INLINE Vec3V_SOA16(ComponentType_arg v): m_x(v),m_y(v),m_z(v) {} // copies xyz into each of the sixteen VectorType's
	VMATH_INLINE Vec3V_SOA16(VectorType_arg v): m_x(v.x()),m_y(v.y()),m_z(v.z()) {} // treats 'v' as three ScalarV's ..
	VMATH_INLINE Vec3V_SOA16(Vec3V_SOA8_arg v0,Vec3V_SOA8_arg v1):
		m_x(v0.x(),v1.x()),
		m_y(v0.y(),v1.y()),
		m_z(v0.z(),v1.z()) {}
	VMATH_INLINE Vec3V_SOA16(Vec3V_SOA4_arg v0,Vec3V_SOA4_arg v1,Vec3V_SOA4_arg v2,Vec3V_SOA4_arg v3):
		m_x(v0.x(),v1.x(),v2.x(),v3.x()),
		m_y(v0.y(),v1.y(),v2.y(),v3.y()),
		m_z(v0.z(),v1.z(),v2.z(),v3.z()) {}
	VMATH_INLINE Vec3V_SOA16(ComponentType_arg x,ComponentType_arg y,ComponentType_arg z): m_x(x),m_y(y),m_z(z) {}
	VMATH_INLINE Vec3V_SOA16(
		VectorType_arg v0,VectorType_arg v1,VectorType_arg v2,VectorType_arg v3,
		VectorType_arg v4,VectorType_arg v5,VectorType_arg v6,VectorType_arg v7,
		VectorType_arg v8,VectorType_arg v9,VectorType_arg vA,VectorType_arg vB,
		VectorType_arg vC,VectorType_arg vD,VectorType_arg vE,VectorType_arg vF)
	{
		Vec4V x0,y0,z0; Mat34VTranspose(x0,y0,z0,v0,v1,v2,v3);
		Vec4V x1,y1,z1; Mat34VTranspose(x1,y1,z1,v4,v5,v6,v7);
		Vec4V x2,y2,z2; Mat34VTranspose(x2,y2,z2,v8,v9,vA,vB);
		Vec4V x3,y3,z3; Mat34VTranspose(x3,y3,z3,vC,vD,vE,vF);
		m_x = ComponentType(x0,x1,x2,x3);
		m_y = ComponentType(y0,y1,y2,y3);
		m_z = ComponentType(z0,z1,z2,z3);
	}
	
	VMATH_INLINE static OutType Zero() { return ComponentType(V_ZERO); }
	VMATH_INLINE static OutType One() { return ComponentType(V_ONE); }
	
	VMATH_INLINE ComponentType_out x() const { return m_x; }
	VMATH_INLINE ComponentType_out y() const { return m_y; }
	VMATH_INLINE ComponentType_out z() const { return m_z; }

	VMATH_INLINE ComponentType& x_ref() { return m_x; }
	VMATH_INLINE ComponentType& y_ref() { return m_y; }
	VMATH_INLINE ComponentType& z_ref() { return m_z; }
	VMATH_INLINE const ComponentType& x_constref() const { return m_x; }
	VMATH_INLINE const ComponentType& y_constref() const { return m_y; }
	VMATH_INLINE const ComponentType& z_constref() const { return m_z; }

	VMATH_INLINE Vec3V_SOA4_out v0() const { return Vec3V_SOA4(x().v0(),y().v0(),z().v0()); }
	VMATH_INLINE Vec3V_SOA4_out v1() const { return Vec3V_SOA4(x().v1(),y().v1(),z().v1()); }
	VMATH_INLINE Vec3V_SOA4_out v2() const { return Vec3V_SOA4(x().v2(),y().v2(),z().v2()); }
	VMATH_INLINE Vec3V_SOA4_out v3() const { return Vec3V_SOA4(x().v3(),y().v3(),z().v3()); }

	VMATH_INLINE Vec3V_SOA8_out v01() const { return Vec3V_SOA8(x().v01(),y().v01(),z().v01()); }
	VMATH_INLINE Vec3V_SOA8_out v23() const { return Vec3V_SOA8(x().v23(),y().v23(),z().v23()); }

	VMATH_INLINE void GetVectors(VectorType v[NumVectors]) const
	{
		Mat43VTranspose(v[0x0],v[0x1],v[0x2],v[0x3],m_x.v0(),m_y.v0(),m_z.v0());
		Mat43VTranspose(v[0x4],v[0x5],v[0x6],v[0x7],m_x.v1(),m_y.v1(),m_z.v1());
		Mat43VTranspose(v[0x8],v[0x9],v[0xA],v[0xB],m_x.v2(),m_y.v2(),m_z.v2());
		Mat43VTranspose(v[0xC],v[0xD],v[0xE],v[0xF],m_x.v3(),m_y.v3(),m_z.v3());
	}
	VMATH_INLINE VectorType_out GetVector(unsigned index) const { return VectorType(m_x[index],m_y[index],m_z[index]); }

	VMATH_INLINE friend OutType operator +(ArgType a) { return a; } // no-op
	VMATH_INLINE friend OutType operator -(ArgType a) { return T(-a.x(),-a.y(),-a.z()); }
	VMATH_INLINE friend OutType operator +(ArgType a,ArgType b) { return T(a.x() + b.x(),a.y() + b.y(),a.z() + b.z()); }
	VMATH_INLINE friend OutType operator -(ArgType a,ArgType b) { return T(a.x() - b.x(),a.y() - b.y(),a.z() - b.z()); }
	VMATH_INLINE friend OutType operator *(ArgType a,ArgType b) { return T(a.x() * b.x(),a.y() * b.y(),a.z() * b.z()); }
	VMATH_INLINE friend OutType operator /(ArgType a,ArgType b) { return T(a.x() / b.x(),a.y() / b.y(),a.z() / b.z()); }
	VMATH_INLINE friend OutType operator *(ArgType a,ComponentType_arg b) { return T(a.x() * b,a.y() * b,a.z() * b); }
	VMATH_INLINE friend OutType operator /(ArgType a,ComponentType_arg b) { return T(a.x() / b,a.y() / b,a.z() / b); }
	VMATH_INLINE friend OutType operator *(ArgType a,ElementType b) { return T(a.x() * b,a.y() * b,a.z() * b); }
	VMATH_INLINE friend OutType operator /(ArgType a,ElementType b) { return T(a.x() / b,a.y() / b,a.z() / b); }
	VMATH_INLINE friend OutType operator *(ElementType a,ArgType b) { return T(a * b.x(),a * b.y(),a * b.z()); }
	VMATH_INLINE friend OutType operator /(ElementType a,ArgType b) { return T(a / b.x(),a / b.y(),a / b.z()); }
	
	VMATH_INLINE T& operator +=(ArgType b) { return(*this = *this + b); }
	VMATH_INLINE T& operator -=(ArgType b) { return(*this = *this - b); }
	VMATH_INLINE T& operator *=(ArgType b) { return(*this = *this * b); }
	VMATH_INLINE T& operator /=(ArgType b) { return(*this = *this / b); }
	VMATH_INLINE T& operator *=(ComponentType_arg b) { return(*this = *this * b); }
	VMATH_INLINE T& operator /=(ComponentType_arg b) { return(*this = *this / b); }
	VMATH_INLINE T& operator *=(ElementType b) { return(*this = *this * b); }
	VMATH_INLINE T& operator /=(ElementType b) { return(*this = *this / b); }
	
#define PER_COMPONENT_FUNC1(func,a) T(func(a.x()),func(a.y()),func(a.z()))
#define PER_COMPONENT_FUNC2(func,a,b) T(func(a.x(),b.x()),func(a.y(),b.y()),func(a.z(),b.z()))
#define PER_COMPONENT_FUNC3(func,a,b,c) T(func(a.x(),b.x(),c.x()),func(a.y(),b.y(),c.y()),func(a.z(),b.z(),c.z()))
#define PER_COMPONENT_FUNC4(func,a,b,c,d) T(func(a.x(),b.x(),c.x(),d.x()),func(a.y(),b.y(),c.y(),d.y()),func(a.z(),b.z(),c.z(),d.z()))
	VMATH_INLINE friend OutType Min(ArgType a,ArgType b) { return PER_COMPONENT_FUNC2(Min,a,b); }
	VMATH_INLINE friend OutType Max(ArgType a,ArgType b) { return PER_COMPONENT_FUNC2(Max,a,b); }
	VMATH_INLINE friend OutType Min(ArgType a,ArgType b,ArgType c) { return PER_COMPONENT_FUNC3(Min,a,b,c); }
	VMATH_INLINE friend OutType Max(ArgType a,ArgType b,ArgType c) { return PER_COMPONENT_FUNC3(Max,a,b,c); }
	VMATH_INLINE friend OutType Min(ArgType a,ArgType b,ArgType c,ArgType d) { return PER_COMPONENT_FUNC4(Min,a,b,c,d); }
	VMATH_INLINE friend OutType Max(ArgType a,ArgType b,ArgType c,ArgType d) { return PER_COMPONENT_FUNC4(Max,a,b,c,d); }
	VMATH_INLINE friend OutType Abs(ArgType a) { return PER_COMPONENT_FUNC1(Abs,a); }
	VMATH_INLINE friend OutType Clamp(ArgType a,ArgType minVal,ArgType maxVal) { return PER_COMPONENT_FUNC3(Clamp,a,minVal,maxVal); }
	VMATH_INLINE friend OutType Saturate(ArgType a) { return PER_COMPONENT_FUNC1(Saturate,a); }
	VMATH_INLINE friend OutType Floor(ArgType a) { return PER_COMPONENT_FUNC1(Floor,a); }
	VMATH_INLINE friend OutType Ceiling(ArgType a) { return PER_COMPONENT_FUNC1(Ceiling,a); }
	VMATH_INLINE friend OutType Truncate(ArgType a) { return PER_COMPONENT_FUNC1(Truncate,a); }
	VMATH_INLINE friend OutType Round(ArgType a) { return PER_COMPONENT_FUNC1(Round,a); }
	VMATH_INLINE friend OutType Frac(ArgType a) { return PER_COMPONENT_FUNC1(Frac,a); }
	VMATH_INLINE friend OutType Sqrt(ArgType a) { return PER_COMPONENT_FUNC1(Sqrt,a); }
	VMATH_INLINE friend OutType Sqr(ArgType a) { return PER_COMPONENT_FUNC1(Sqr,a); }
	VMATH_INLINE friend OutType Recip(ArgType a) { return PER_COMPONENT_FUNC1(Recip,a); }
	VMATH_INLINE friend OutType RecipSqrt(ArgType a) { return PER_COMPONENT_FUNC1(RecipSqrt,a); }
#undef PER_COMPONENT_FUNC1
#undef PER_COMPONENT_FUNC2
#undef PER_COMPONENT_FUNC3
#undef PER_COMPONENT_FUNC4

	VMATH_INLINE friend ComponentType_out MinElement(ArgType a) { return Min(a.x(),a.y(),a.z()); }
	VMATH_INLINE friend ComponentType_out MaxElement(ArgType a) { return Max(a.x(),a.y(),a.z()); }

	VMATH_INLINE friend ComponentType_out Dot(ArgType a,ArgType b) { return MultiplyAdd(a.x(),b.x(),a.y(),b.y(),a.z(),b.z()); }
	VMATH_INLINE friend ComponentType_out MagSqr(ArgType a) { return Dot(a,a); }
	VMATH_INLINE friend ComponentType_out Mag(ArgType a) { return Sqrt(MagSqr(a)); }

	VMATH_INLINE friend OutType Cross(ArgType a,ArgType b)
	{
		return T(
			MultiplySub(a.y(),b.z(),b.y(),a.z()),  // a.y*b.z - b.y*a.z
			MultiplySub(a.z(),b.x(),b.z(),a.x()),  // a.z*b.x - b.z*a.x
			MultiplySub(a.x(),b.y(),b.x(),a.y())); // a.x*b.y - b.x*a.y
	}

	VMATH_INLINE friend OutType Normalize(ArgType a) { return a*RecipSqrt(MagSqr(a)); }

	VMATH_INLINE friend OutType Select(ArgType a,ArgType b,ArgType sel)
	{
		return T(
			Select(a.x(),b.x(),ComponentType::BoolV(sel.x())),
			Select(a.y(),b.y(),ComponentType::BoolV(sel.y())),
			Select(a.z(),b.z(),ComponentType::BoolV(sel.z())));
	}

	ComponentType m_x,m_y,m_z;
};

typedef typename Vec3V_SOA16::ArgType Vec3V_SOA16_arg;
typedef typename Vec3V_SOA16::OutType Vec3V_SOA16_out;

template <> class SOA_T<16>
{
public:
	typedef Vec3V_SOA16 Vec3V_SOAType;
};
#endif // HAS_VEC16V
#endif // HAS_VEC8V

#endif // _INCLUDE_VMATH_SOA_H_

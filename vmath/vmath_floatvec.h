// =============================
// common/vmath/vmath_floatvec.h
// =============================

#ifndef _INCLUDE_VMATH_FLOATVEC_H_
#define _INCLUDE_VMATH_FLOATVEC_H_

#include "vmath_common.h"

template <typename ElementType> class Vec2_T
{
public:
	typedef Vec2_T<ElementType> T;
	typedef const T& ArgType;
	typedef const T OutType;
	enum { NumElements = 2 };

	VMATH_INLINE Vec2_T() {}
	VMATH_INLINE explicit Vec2_T(ElementType f): m_x(f),m_y(f) {}
	VMATH_INLINE Vec2_T(ElementType x,ElementType y): m_x(x),m_y(y) {}
	VMATH_INLINE Vec2_T(VectorConstantInitializer constant)
	{
		StaticAssert(sizeof(this) == sizeof(ElementType)*NumElements);
		switch (constant) {
		case V_ZERO: m_x = m_y = static_cast<ElementType>(0); break;
		case V_ONE: m_x = m_y = static_cast<ElementType>(1); break;
		case V_XAXIS: m_x = static_cast<ElementType>(1), m_y = static_cast<ElementType>(0); break;
		case V_YAXIS: m_y = static_cast<ElementType>(1), m_x = static_cast<ElementType>(0); break;
		default: DEBUG_ASSERT(false);
		}
	}

	VMATH_INLINE ElementType x() const { return m_x; }
	VMATH_INLINE ElementType y() const { return m_y; }

	VMATH_INLINE float xf() const { return static_cast<float>(m_x); }
	VMATH_INLINE float yf() const { return static_cast<float>(m_y); }

	VMATH_INLINE ElementType& x_ref() { return m_x; }
	VMATH_INLINE ElementType& y_ref() { return m_y; }
	VMATH_INLINE const ElementType& x_constref() const { return m_x; }
	VMATH_INLINE const ElementType& y_constref() const { return m_y; }

	VMATH_INLINE ElementType operator [](unsigned index) const { DEBUG_ASSERT(index < NumElements); return reinterpret_cast<const ElementType*>(this)[index]; }
	VMATH_INLINE ElementType& operator [](unsigned index) { DEBUG_ASSERT(index < NumElements); return reinterpret_cast<ElementType*>(this)[index]; }

	VMATH_INLINE friend OutType operator +(ArgType a) { return a; } // no-op
	VMATH_INLINE friend OutType operator -(ArgType a) { return T(-a.x(),-a.y()); }
	VMATH_INLINE friend OutType operator +(ArgType a,ArgType b) { return T(a.x() + b.x(),a.y() + b.y()); }
	VMATH_INLINE friend OutType operator -(ArgType a,ArgType b) { return T(a.x() - b.x(),a.y() - b.y()); }
	VMATH_INLINE friend OutType operator *(ArgType a,ArgType b) { return T(a.x() * b.x(),a.y() * b.y()); }
	VMATH_INLINE friend OutType operator /(ArgType a,ArgType b) { return T(a.x() / b.x(),a.y() / b.y()); }
	VMATH_INLINE friend OutType operator *(ArgType a,ElementType b) { return T(a.x() * b,a.y() * b); }
	VMATH_INLINE friend OutType operator /(ArgType a,ElementType b) { return T(a.x() / b,a.y() / b); }
	VMATH_INLINE friend OutType operator *(ElementType a,ArgType b) { return T(a * b.x(),a * b.y()); }
	VMATH_INLINE friend OutType operator /(ElementType a,ArgType b) { return T(a / b.x(),a / b.y()); }

	VMATH_INLINE T& operator +=(ArgType b) { return(*this = *this + b); }
	VMATH_INLINE T& operator -=(ArgType b) { return(*this = *this - b); }
	VMATH_INLINE T& operator *=(ArgType b) { return(*this = *this * b); }
	VMATH_INLINE T& operator /=(ArgType b) { return(*this = *this / b); }
	VMATH_INLINE T& operator *=(ElementType b) { return(*this = *this * b); }
	VMATH_INLINE T& operator /=(ElementType b) { return(*this = *this / b); }

#define PER_COMPONENT_FUNC1(func,a) T(func(a.x()),func(a.y()))
#define PER_COMPONENT_FUNC2(func,a,b) T(func(a.x(),b.x()),func(a.y(),b.y()))
#define PER_COMPONENT_FUNC3(func,a,b,c) T(func(a.x(),b.x(),c.x()),func(a.y(),b.y(),c.y()))
#define PER_COMPONENT_FUNC4(func,a,b,c,d) T(func(a.x(),b.x(),c.x(),d.x()),func(a.y(),b.y(),c.y(),d.y()))
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

	VMATH_INLINE friend OutType MultiplyAdd(ArgType a,ArgType b,ArgType c) { return T(a.x()*b.x() + c.x(),a.y()*b.y() + c.y()); } // a*b + c
	VMATH_INLINE friend OutType MultiplySub(ArgType a,ArgType b,ArgType c) { return T(a.x()*b.x() - c.x(),a.y()*b.y() - c.y()); } // a*b - c
	VMATH_INLINE friend OutType NegMultiplyAdd(ArgType a,ArgType b,ArgType c) { return T(c.x() - a.x()*b.x(),c.y() - a.y()*b.y()); } // -(a*b - c) = c - a*b
	VMATH_INLINE friend OutType NegMultiplySub(ArgType a,ArgType b,ArgType c) { return T(-(a.x()*b.x() + c.x()),-(a.y()*b.y() + c.y())); } // -(a*b + c)
	VMATH_INLINE friend OutType MultiplyAdd(ArgType a1,ArgType b1,ArgType a2,ArgType b2) { return MultiplyAdd(a1,b1,a2*b2); } // a1*b1 + a2*b2
	VMATH_INLINE friend OutType MultiplySub(ArgType a1,ArgType b1,ArgType a2,ArgType b2) { return MultiplySub(a1,b1,a2*b2); } // a1*b1 - a2*b2
	VMATH_INLINE friend OutType MultiplyAdd(ArgType a1,ArgType b1,ArgType a2,ArgType b2,ArgType c) { return MultiplyAdd(a1,b1,MultiplyAdd(a2,b2,c)); } // a1*b1 + a2*b2 + c
	VMATH_INLINE friend OutType MultiplyAdd(ArgType a1,ArgType b1,ArgType a2,ArgType b2,ArgType a3,ArgType b3) { return MultiplyAdd(a1,b1,MultiplyAdd(a2,b2,a3*b3)); } // a1*b1 + a2*b2 + a3*b3
	VMATH_INLINE friend OutType MultiplyAdd(ArgType a1,ArgType b1,ArgType a2,ArgType b2,ArgType a3,ArgType b3,ArgType c) { return MultiplyAdd(a1,b1,MultiplyAdd(a2,b2,MultiplyAdd(a3,b3,c))); } // a1*b1 + a2*b2 + a3*b3 + c
	VMATH_INLINE friend OutType MultiplyAdd(ArgType a1,ArgType b1,ArgType a2,ArgType b2,ArgType a3,ArgType b3,ArgType a4,ArgType b4) { return MultiplyAdd(a1,b1,MultiplyAdd(a2,b2,MultiplyAdd(a3,b3,a4*b4))); } // a1*b1 + a2*b2 + a3*b3 + a3*b4

	VMATH_INLINE friend ElementType MinElement(ArgType a) { return Min(a.x(),a.y()); }
	VMATH_INLINE friend ElementType MaxElement(ArgType a) { return Max(a.x(),a.y()); }

	VMATH_INLINE friend ElementType Dot(ArgType a,ArgType b) { return MultiplyAdd(a.x(),b.x(),a.y(),b.y()); }
	VMATH_INLINE friend ElementType MagSqr(ArgType a) { return Dot(a,a); }
	VMATH_INLINE friend ElementType Mag(ArgType a) { return Sqrt(MagSqr(a)); }
	
	VMATH_INLINE friend ElementType Cross(ArgType a,ArgType b) { return MultiplySub(a.x(),b.y(),b.x(),a.y()); } // a.x*b.y - b.x*a.y

	VMATH_INLINE friend OutType Normalize(ArgType a) { return a*RecipSqrt(MagSqr(a)); }

	VMATH_INLINE static OutType SinCos(ElementType angle) { return T(Cos(angle),Sin(angle)); }

	class BoolV
	{
	public:
		typedef const BoolV& ArgType;

		VMATH_INLINE BoolV(bool x,bool y): m_x(x),m_y(y) {}

		VMATH_INLINE friend const BoolV operator ==(ArgType a,ArgType b) { return BoolV(a.x() == b.x(),a.y() == b.y()); }
		VMATH_INLINE friend const BoolV operator !=(ArgType a,ArgType b) { return BoolV(a.x() != b.x(),a.y() != b.y()); }
		VMATH_INLINE friend const BoolV operator &&(ArgType a,ArgType b) { return BoolV(a.x() && b.x(),a.y() && b.y()); }
		VMATH_INLINE friend const BoolV operator ||(ArgType a,ArgType b) { return BoolV(a.x() || b.x(),a.y() || b.y()); }
		VMATH_INLINE friend const BoolV operator !(ArgType a) { return BoolV(!a.x(),!a.y()); }

		bool m_x,m_y;
	};
	VMATH_INLINE friend const BoolV operator ==(ArgType a,ArgType b) { return BoolV(a.x() == b.x(),a.y() == b.y()); }
	VMATH_INLINE friend const BoolV operator !=(ArgType a,ArgType b) { return BoolV(a.x() != b.x(),a.y() != b.y()); }
	VMATH_INLINE friend const BoolV operator < (ArgType a,ArgType b) { return BoolV(a.x() <  b.x(),a.y() <  b.y()); }
	VMATH_INLINE friend const BoolV operator <=(ArgType a,ArgType b) { return BoolV(a.x() <= b.x(),a.y() <= b.y()); }
	VMATH_INLINE friend const BoolV operator > (ArgType a,ArgType b) { return BoolV(a.x() >  b.x(),a.y() >  b.y()); }
	VMATH_INLINE friend const BoolV operator >=(ArgType a,ArgType b) { return BoolV(a.x() >= b.x(),a.y() >= b.y()); }
	
	VMATH_INLINE friend const BoolV operator ==(ArgType a,ElementType b) { return operator ==(a,T(b)); }
	VMATH_INLINE friend const BoolV operator !=(ArgType a,ElementType b) { return operator !=(a,T(b)); }
	VMATH_INLINE friend const BoolV operator < (ArgType a,ElementType b) { return operator < (a,T(b)); }
	VMATH_INLINE friend const BoolV operator <=(ArgType a,ElementType b) { return operator <=(a,T(b)); }
	VMATH_INLINE friend const BoolV operator > (ArgType a,ElementType b) { return operator > (a,T(b)); }
	VMATH_INLINE friend const BoolV operator >=(ArgType a,ElementType b) { return operator >=(a,T(b)); }

	VMATH_INLINE friend const BoolV operator ==(ElementType a,ArgType b) { return operator ==(T(a),b); }
	VMATH_INLINE friend const BoolV operator !=(ElementType a,ArgType b) { return operator !=(T(a),b); }
	VMATH_INLINE friend const BoolV operator < (ElementType a,ArgType b) { return operator < (T(a),b); }
	VMATH_INLINE friend const BoolV operator <=(ElementType a,ArgType b) { return operator <=(T(a),b); }
	VMATH_INLINE friend const BoolV operator > (ElementType a,ArgType b) { return operator > (T(a),b); }
	VMATH_INLINE friend const BoolV operator >=(ElementType a,ArgType b) { return operator >=(T(a),b); }

	VMATH_INLINE friend T Select(ArgType a,ArgType b,typename BoolV::ArgType sel)
	{
		return T(
			sel.m_x ? b.x() : a.x(),
			sel.m_y ? b.y() : a.y());
	}

	VMATH_INLINE friend OutType Select00(ArgType arg0,ArgType arg1) { return arg0; }
	VMATH_INLINE friend OutType Select01(ArgType arg0,ArgType arg1) { return T(arg0.x(),arg1.y()); }
	VMATH_INLINE friend OutType Select10(ArgType arg0,ArgType arg1) { return T(arg1.x(),arg0.y()); }
	VMATH_INLINE friend OutType Select11(ArgType arg0,ArgType arg1) { return arg1; }

private:
	ElementType m_x,m_y;
};

template <typename ElementType> class Vec3_T
{
public:
	typedef Vec3_T<ElementType> T;
	typedef const T& ArgType;
	typedef const T OutType;
	enum { NumElements = 3 };

	VMATH_INLINE Vec3_T() {}
	VMATH_INLINE explicit Vec3_T(ElementType f): m_x(f),m_y(f),m_z(f) {}
	VMATH_INLINE Vec3_T(ElementType x,ElementType y,ElementType z): m_x(x),m_y(y),m_z(z) {}
	VMATH_INLINE Vec3_T(VectorConstantInitializer constant)
	{
		StaticAssert(sizeof(this) == sizeof(ElementType)*NumElements);
		switch (constant) {
		case V_ZERO: m_x = m_y = m_z = static_cast<ElementType>(0); break;
		case V_ONE: m_x = m_y = m_z = static_cast<ElementType>(1); break;
		case V_XAXIS: m_x = static_cast<ElementType>(1), m_y = m_z = static_cast<ElementType>(0); break;
		case V_YAXIS: m_y = static_cast<ElementType>(1), m_x = m_z = static_cast<ElementType>(0); break;
		case V_ZAXIS: m_z = static_cast<ElementType>(1), m_x = m_y = static_cast<ElementType>(0); break;
		default: DEBUG_ASSERT(false);
		}
	}

	VMATH_INLINE ElementType x() const { return m_x; }
	VMATH_INLINE ElementType y() const { return m_y; }
	VMATH_INLINE ElementType z() const { return m_z; }

	VMATH_INLINE float xf() const { return static_cast<float>(m_x); }
	VMATH_INLINE float yf() const { return static_cast<float>(m_y); }
	VMATH_INLINE float zf() const { return static_cast<float>(m_z); }

	VMATH_INLINE ElementType& x_ref() { return m_x; }
	VMATH_INLINE ElementType& y_ref() { return m_y; }
	VMATH_INLINE ElementType& z_ref() { return m_z; }
	VMATH_INLINE const ElementType& x_constref() const { return m_x; }
	VMATH_INLINE const ElementType& y_constref() const { return m_y; }
	VMATH_INLINE const ElementType& z_constref() const { return m_z; }

	VMATH_INLINE typename Vec2_T<ElementType>::OutType xy() const { return Vec2_T<ElementType>(x(),y()); }

	VMATH_INLINE Vec2_T<ElementType>& xy_ref() { return *reinterpret_cast<Vec2_T<ElementType>*>(this); }

	VMATH_INLINE ElementType operator [](unsigned index) const { DEBUG_ASSERT(index < NumElements); return reinterpret_cast<const ElementType*>(this)[index]; }
	VMATH_INLINE ElementType& operator [](unsigned index) { DEBUG_ASSERT(index < NumElements); return reinterpret_cast<ElementType*>(this)[index]; }

	VMATH_INLINE friend OutType operator +(ArgType a) { return a; } // no-op
	VMATH_INLINE friend OutType operator -(ArgType a) { return T(-a.x(),-a.y(),-a.z()); }
	VMATH_INLINE friend OutType operator +(ArgType a,ArgType b) { return T(a.x() + b.x(),a.y() + b.y(),a.z() + b.z()); }
	VMATH_INLINE friend OutType operator -(ArgType a,ArgType b) { return T(a.x() - b.x(),a.y() - b.y(),a.z() - b.z()); }
	VMATH_INLINE friend OutType operator *(ArgType a,ArgType b) { return T(a.x() * b.x(),a.y() * b.y(),a.z() * b.z()); }
	VMATH_INLINE friend OutType operator /(ArgType a,ArgType b) { return T(a.x() / b.x(),a.y() / b.y(),a.z() / b.z()); }
	VMATH_INLINE friend OutType operator *(ArgType a,ElementType b) { return T(a.x() * b,a.y() * b,a.z() * b); }
	VMATH_INLINE friend OutType operator /(ArgType a,ElementType b) { return T(a.x() / b,a.y() / b,a.z() / b); }
	VMATH_INLINE friend OutType operator *(ElementType a,ArgType b) { return T(a * b.x(),a * b.y(),a * b.z()); }
	VMATH_INLINE friend OutType operator /(ElementType a,ArgType b) { return T(a / b.x(),a / b.y(),a / b.z()); }

	VMATH_INLINE T& operator +=(ArgType b) { return(*this = *this + b); }
	VMATH_INLINE T& operator -=(ArgType b) { return(*this = *this - b); }
	VMATH_INLINE T& operator *=(ArgType b) { return(*this = *this * b); }
	VMATH_INLINE T& operator /=(ArgType b) { return(*this = *this / b); }
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

	VMATH_INLINE friend OutType MultiplyAdd(ArgType a,ArgType b,ArgType c) { return T(a.x()*b.x() + c.x(),a.y()*b.y() + c.y(),a.z()*b.z() + c.z()); } // a*b + c
	VMATH_INLINE friend OutType MultiplySub(ArgType a,ArgType b,ArgType c) { return T(a.x()*b.x() - c.x(),a.y()*b.y() - c.y(),a.z()*b.z() - c.z()); } // a*b - c
	VMATH_INLINE friend OutType NegMultiplyAdd(ArgType a,ArgType b,ArgType c) { return T(c.x() - a.x()*b.x(),c.y() - a.y()*b.y(),c.z() - a.z()*b.z()); } // -(a*b - c) = c - a*b
	VMATH_INLINE friend OutType NegMultiplySub(ArgType a,ArgType b,ArgType c) { return T(-(a.x()*b.x() + c.x()),-(a.y()*b.y() + c.y()),-(a.z()*b.z() + c.z())); } // -(a*b + c)
	VMATH_INLINE friend OutType MultiplyAdd(ArgType a1,ArgType b1,ArgType a2,ArgType b2) { return MultiplyAdd(a1,b1,a2*b2); } // a1*b1 + a2*b2
	VMATH_INLINE friend OutType MultiplySub(ArgType a1,ArgType b1,ArgType a2,ArgType b2) { return MultiplySub(a1,b1,a2*b2); } // a1*b1 - a2*b2
	VMATH_INLINE friend OutType MultiplyAdd(ArgType a1,ArgType b1,ArgType a2,ArgType b2,ArgType c) { return MultiplyAdd(a1,b1,MultiplyAdd(a2,b2,c)); } // a1*b1 + a2*b2 + c
	VMATH_INLINE friend OutType MultiplyAdd(ArgType a1,ArgType b1,ArgType a2,ArgType b2,ArgType a3,ArgType b3) { return MultiplyAdd(a1,b1,MultiplyAdd(a2,b2,a3*b3)); } // a1*b1 + a2*b2 + a3*b3
	VMATH_INLINE friend OutType MultiplyAdd(ArgType a1,ArgType b1,ArgType a2,ArgType b2,ArgType a3,ArgType b3,ArgType c) { return MultiplyAdd(a1,b1,MultiplyAdd(a2,b2,MultiplyAdd(a3,b3,c))); } // a1*b1 + a2*b2 + a3*b3 + c
	VMATH_INLINE friend OutType MultiplyAdd(ArgType a1,ArgType b1,ArgType a2,ArgType b2,ArgType a3,ArgType b3,ArgType a4,ArgType b4) { return MultiplyAdd(a1,b1,MultiplyAdd(a2,b2,MultiplyAdd(a3,b3,a4*b4))); } // a1*b1 + a2*b2 + a3*b3 + a3*b4

	VMATH_INLINE friend ElementType MinElement(ArgType a) { return Min(a.x(),a.y(),a.z()); }
	VMATH_INLINE friend ElementType MaxElement(ArgType a) { return Max(a.x(),a.y(),a.z()); }

	VMATH_INLINE friend ElementType Dot(ArgType a,ArgType b) { return MultiplyAdd(a.x(),b.x(),a.y(),b.y(),a.z(),b.z()); }
	VMATH_INLINE friend ElementType MagSqr(ArgType a) { return Dot(a,a); }
	VMATH_INLINE friend ElementType Mag(ArgType a) { return Sqrt(MagSqr(a)); }

	VMATH_INLINE friend ElementType Cross(ArgType a,ArgType b)
	{
		return T(
			MultiplySub(a.y(),b.z(),b.y(),a.z()),  // a.y*b.z - b.y*a.z
			MultiplySub(a.z(),b.x(),b.z(),a.x()),  // a.z*b.x - b.z*a.x
			MultiplySub(a.x(),b.y(),b.x(),a.y())); // a.x*b.y - b.x*a.y
	}

	VMATH_INLINE friend OutType Normalize(ArgType a) { return a*RecipSqrt(MagSqr(a)); }

	class BoolV
	{
	public:
		typedef const BoolV& ArgType;

		VMATH_INLINE BoolV(bool x,bool y,bool z): m_x(x),m_y(y),m_z(z) {}

		VMATH_INLINE friend const BoolV operator ==(ArgType a,ArgType b) { return BoolV(a.x() == b.x(),a.y() == b.y(),a.z() == b.z()); }
		VMATH_INLINE friend const BoolV operator !=(ArgType a,ArgType b) { return BoolV(a.x() != b.x(),a.y() != b.y(),a.z() != b.z()); }
		VMATH_INLINE friend const BoolV operator &&(ArgType a,ArgType b) { return BoolV(a.x() && b.x(),a.y() && b.y(),a.z() && b.z()); }
		VMATH_INLINE friend const BoolV operator ||(ArgType a,ArgType b) { return BoolV(a.x() || b.x(),a.y() || b.y(),a.z() || b.z()); }
		VMATH_INLINE friend const BoolV operator !(ArgType a) { return BoolV(!a.x(),!a.y(),!a.z()); }

		bool m_x,m_y,m_z;
	};
	VMATH_INLINE friend const BoolV operator ==(ArgType a,ArgType b) { return BoolV(a.x() == b.x(),a.y() == b.y(),a.z() == b.z()); }
	VMATH_INLINE friend const BoolV operator !=(ArgType a,ArgType b) { return BoolV(a.x() != b.x(),a.y() != b.y(),a.z() != b.z()); }
	VMATH_INLINE friend const BoolV operator < (ArgType a,ArgType b) { return BoolV(a.x() <  b.x(),a.y() <  b.y(),a.z() <  b.z()); }
	VMATH_INLINE friend const BoolV operator <=(ArgType a,ArgType b) { return BoolV(a.x() <= b.x(),a.y() <= b.y(),a.z() <= b.z()); }
	VMATH_INLINE friend const BoolV operator > (ArgType a,ArgType b) { return BoolV(a.x() >  b.x(),a.y() >  b.y(),a.z() >  b.z()); }
	VMATH_INLINE friend const BoolV operator >=(ArgType a,ArgType b) { return BoolV(a.x() >= b.x(),a.y() >= b.y(),a.z() >= b.z()); }

	VMATH_INLINE friend const BoolV operator ==(ArgType a,ElementType b) { return operator ==(a,T(b)); }
	VMATH_INLINE friend const BoolV operator !=(ArgType a,ElementType b) { return operator !=(a,T(b)); }
	VMATH_INLINE friend const BoolV operator < (ArgType a,ElementType b) { return operator < (a,T(b)); }
	VMATH_INLINE friend const BoolV operator <=(ArgType a,ElementType b) { return operator <=(a,T(b)); }
	VMATH_INLINE friend const BoolV operator > (ArgType a,ElementType b) { return operator > (a,T(b)); }
	VMATH_INLINE friend const BoolV operator >=(ArgType a,ElementType b) { return operator >=(a,T(b)); }

	VMATH_INLINE friend const BoolV operator ==(ElementType a,ArgType b) { return operator ==(T(a),b); }
	VMATH_INLINE friend const BoolV operator !=(ElementType a,ArgType b) { return operator !=(T(a),b); }
	VMATH_INLINE friend const BoolV operator < (ElementType a,ArgType b) { return operator < (T(a),b); }
	VMATH_INLINE friend const BoolV operator <=(ElementType a,ArgType b) { return operator <=(T(a),b); }
	VMATH_INLINE friend const BoolV operator > (ElementType a,ArgType b) { return operator > (T(a),b); }
	VMATH_INLINE friend const BoolV operator >=(ElementType a,ArgType b) { return operator >=(T(a),b); }	

	VMATH_INLINE friend OutType Select(ArgType a,ArgType b,typename BoolV::ArgType sel)
	{
		return T(
			sel.m_x ? b.x() : a.x(),
			sel.m_y ? b.y() : a.y(),
			sel.m_z ? b.z() : a.z());
	}

	VMATH_INLINE friend OutType Select000(ArgType arg0,ArgType arg1) { return arg0; }
	VMATH_INLINE friend OutType Select001(ArgType arg0,ArgType arg1) { return T(arg0.x(),arg0.y(),arg1.z()); }
	VMATH_INLINE friend OutType Select010(ArgType arg0,ArgType arg1) { return T(arg0.x(),arg1.y(),arg0.z()); }
	VMATH_INLINE friend OutType Select011(ArgType arg0,ArgType arg1) { return T(arg0.x(),arg1.y(),arg1.z()); }
	VMATH_INLINE friend OutType Select100(ArgType arg0,ArgType arg1) { return T(arg1.x(),arg0.y(),arg0.z()); }
	VMATH_INLINE friend OutType Select101(ArgType arg0,ArgType arg1) { return T(arg1.x(),arg0.y(),arg1.z()); }
	VMATH_INLINE friend OutType Select110(ArgType arg0,ArgType arg1) { return T(arg1.x(),arg1.y(),arg0.z()); }
	VMATH_INLINE friend OutType Select111(ArgType arg0,ArgType arg1) { return arg1; }

private:
	ElementType m_x,m_y,m_z;
};

template <typename ElementType> class Vec4_T
{
public:
	typedef Vec4_T<ElementType> T;
	typedef const T& ArgType;
	typedef const T OutType;
	enum { NumElements = 4 };

	VMATH_INLINE Vec4_T() {}
	VMATH_INLINE explicit Vec4_T(ElementType f): m_x(f),m_y(f),m_z(f),m_w(f) {}
	VMATH_INLINE Vec4_T(ElementType x,ElementType y,ElementType z,ElementType w): m_x(x),m_y(y),m_z(z),m_w(w) {}
	VMATH_INLINE Vec4_T(VectorConstantInitializer constant)
	{
		StaticAssert(sizeof(this) == sizeof(ElementType)*NumElements);
		switch (constant) {
		case V_ZERO: m_x = m_y = m_z = m_w = static_cast<ElementType>(0); break;
		case V_ONE: m_x = m_y = m_z = m_w = static_cast<ElementType>(1); break;
		case V_XAXIS: m_x = static_cast<ElementType>(1), m_y = m_z = m_w = static_cast<ElementType>(0); break;
		case V_YAXIS: m_y = static_cast<ElementType>(1), m_x = m_z = m_w = static_cast<ElementType>(0); break;
		case V_ZAXIS: m_z = static_cast<ElementType>(1), m_x = m_y = m_w = static_cast<ElementType>(0); break;
		case V_WAXIS: m_w = static_cast<ElementType>(1), m_x = m_y = m_z = static_cast<ElementType>(0); break;
		default: DEBUG_ASSERT(false);
		}
	}

	VMATH_INLINE ElementType x() const { return m_x; }
	VMATH_INLINE ElementType y() const { return m_y; }
	VMATH_INLINE ElementType z() const { return m_z; }
	VMATH_INLINE ElementType w() const { return m_w; }

	VMATH_INLINE float xf() const { return static_cast<float>(m_x); }
	VMATH_INLINE float yf() const { return static_cast<float>(m_y); }
	VMATH_INLINE float zf() const { return static_cast<float>(m_z); }
	VMATH_INLINE float wf() const { return static_cast<float>(m_w); }

	VMATH_INLINE ElementType& x_ref() { return m_x; }
	VMATH_INLINE ElementType& y_ref() { return m_y; }
	VMATH_INLINE ElementType& z_ref() { return m_z; }
	VMATH_INLINE ElementType& w_ref() { return m_w; }
	VMATH_INLINE const ElementType& x_constref() const { return m_x; }
	VMATH_INLINE const ElementType& y_constref() const { return m_y; }
	VMATH_INLINE const ElementType& z_constref() const { return m_z; }
	VMATH_INLINE const ElementType& w_constref() const { return m_w; }

	VMATH_INLINE typename Vec2_T<ElementType>::OutType xy() const { return Vec2_T<ElementType>(x(),y()); }
	VMATH_INLINE typename Vec3_T<ElementType>::OutType xyz() const  { return Vec3_T<ElementType>(x(),y(),z()); }

	VMATH_INLINE Vec2_T<ElementType>& xy_ref() { return *reinterpret_cast<Vec2_T<ElementType>*>(this); }
	VMATH_INLINE Vec3_T<ElementType>& xyz_ref() { return *reinterpret_cast<Vec3_T<ElementType>*>(this); }

	VMATH_INLINE ElementType operator [](unsigned index) const { DEBUG_ASSERT(index < NumElements); return reinterpret_cast<const ElementType*>(this)[index]; }
	VMATH_INLINE ElementType& operator [](unsigned index) { DEBUG_ASSERT(index < NumElements); return reinterpret_cast<ElementType*>(this)[index]; }

	VMATH_INLINE friend OutType operator +(ArgType a) { return a; } // no-op
	VMATH_INLINE friend OutType operator -(ArgType a) { return T(-a.x(),-a.y(),-a.z()); }
	VMATH_INLINE friend OutType operator +(ArgType a,ArgType b) { return T(a.x() + b.x(),a.y() + b.y(),a.z() + b.z(),a.w() + b.w()); }
	VMATH_INLINE friend OutType operator -(ArgType a,ArgType b) { return T(a.x() - b.x(),a.y() - b.y(),a.z() - b.z(),a.w() - b.w()); }
	VMATH_INLINE friend OutType operator *(ArgType a,ArgType b) { return T(a.x() * b.x(),a.y() * b.y(),a.z() * b.z(),a.w() * b.w()); }
	VMATH_INLINE friend OutType operator /(ArgType a,ArgType b) { return T(a.x() / b.x(),a.y() / b.y(),a.z() / b.z(),a.w() / b.w()); }
	VMATH_INLINE friend OutType operator *(ArgType a,ElementType b) { return T(a.x() * b,a.y() * b,a.z() * b,a.w() * b); }
	VMATH_INLINE friend OutType operator /(ArgType a,ElementType b) { return T(a.x() / b,a.y() / b,a.z() / b,a.w() / b); }
	VMATH_INLINE friend OutType operator *(ElementType a,ArgType b) { return T(a * b.x(),a * b.y(),a * b.z(),a * b.w()); }
	VMATH_INLINE friend OutType operator /(ElementType a,ArgType b) { return T(a / b.x(),a / b.y(),a / b.z(),a / b.w()); }

	VMATH_INLINE T& operator +=(ArgType b) { return(*this = *this + b); }
	VMATH_INLINE T& operator -=(ArgType b) { return(*this = *this - b); }
	VMATH_INLINE T& operator *=(ArgType b) { return(*this = *this * b); }
	VMATH_INLINE T& operator /=(ArgType b) { return(*this = *this / b); }
	VMATH_INLINE T& operator *=(ElementType b) { return(*this = *this * b); }
	VMATH_INLINE T& operator /=(ElementType b) { return(*this = *this / b); }

#define PER_COMPONENT_FUNC1(func,a) T(func(a.x()),func(a.y()),func(a.z()),func(a.w()))
#define PER_COMPONENT_FUNC2(func,a,b) T(func(a.x(),b.x()),func(a.y(),b.y()),func(a.z(),b.z()),func(a.w(),b.w()))
#define PER_COMPONENT_FUNC3(func,a,b,c) T(func(a.x(),b.x(),c.x()),func(a.y(),b.y(),c.y()),func(a.z(),b.z(),c.z()),func(a.w(),b.w(),c.w()))
#define PER_COMPONENT_FUNC4(func,a,b,c,d) T(func(a.x(),b.x(),c.x(),d.x()),func(a.y(),b.y(),c.y(),d.y()),func(a.z(),b.z(),c.z(),d.z()),func(a.w(),b.w(),c.w(),d.w()))
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

	VMATH_INLINE friend OutType MultiplyAdd(ArgType a,ArgType b,ArgType c) { return T(a.x()*b.x() + c.x(),a.y()*b.y() + c.y(),a.z()*b.z() + c.z(),a.w()*b.w() + c.w()); } // a*b + c
	VMATH_INLINE friend OutType MultiplySub(ArgType a,ArgType b,ArgType c) { return T(a.x()*b.x() - c.x(),a.y()*b.y() - c.y(),a.z()*b.z() - c.z(),a.w()*b.w() - c.w()); } // a*b - c
	VMATH_INLINE friend OutType NegMultiplyAdd(ArgType a,ArgType b,ArgType c) { return T(c.x() - a.x()*b.x(),c.y() - a.y()*b.y(),c.z() - a.z()*b.z(),c.w() - a.w()*b.w()); }// -(a*b - c) = c - a*b
	VMATH_INLINE friend OutType NegMultiplySub(ArgType a,ArgType b,ArgType c) { return T(-(a.x()*b.x() + c.x()),-(a.y()*b.y() + c.y()),-(a.z()*b.z() + c.z()),-(a.w()*b.w() + c.w())); } // -(a*b + c)
	VMATH_INLINE friend OutType MultiplyAdd(ArgType a1,ArgType b1,ArgType a2,ArgType b2) { return MultiplyAdd(a1,b1,a2*b2); } // a1*b1 + a2*b2
	VMATH_INLINE friend OutType MultiplySub(ArgType a1,ArgType b1,ArgType a2,ArgType b2) { return MultiplySub(a1,b1,a2*b2); } // a1*b1 - a2*b2
	VMATH_INLINE friend OutType MultiplyAdd(ArgType a1,ArgType b1,ArgType a2,ArgType b2,ArgType c) { return MultiplyAdd(a1,b1,MultiplyAdd(a2,b2,c)); } // a1*b1 + a2*b2 + c
	VMATH_INLINE friend OutType MultiplyAdd(ArgType a1,ArgType b1,ArgType a2,ArgType b2,ArgType a3,ArgType b3) { return MultiplyAdd(a1,b1,MultiplyAdd(a2,b2,a3*b3)); } // a1*b1 + a2*b2 + a3*b3
	VMATH_INLINE friend OutType MultiplyAdd(ArgType a1,ArgType b1,ArgType a2,ArgType b2,ArgType a3,ArgType b3,ArgType c) { return MultiplyAdd(a1,b1,MultiplyAdd(a2,b2,MultiplyAdd(a3,b3,c))); } // a1*b1 + a2*b2 + a3*b3 + c
	VMATH_INLINE friend OutType MultiplyAdd(ArgType a1,ArgType b1,ArgType a2,ArgType b2,ArgType a3,ArgType b3,ArgType a4,ArgType b4) { return MultiplyAdd(a1,b1,MultiplyAdd(a2,b2,MultiplyAdd(a3,b3,a4*b4))); } // a1*b1 + a2*b2 + a3*b3 + a3*b4

	VMATH_INLINE friend ElementType MinElement(ArgType a) { return Min(a.x(),a.y(),a.z(),a.w()); }
	VMATH_INLINE friend ElementType MaxElement(ArgType a) { return Max(a.x(),a.y(),a.z(),a.w()); }

	VMATH_INLINE friend ElementType Dot(ArgType a,ArgType b) { return MultiplyAdd(a.x(),b.x(),a.y(),b.y(),a.z(),b.z(),a.w(),b.w()); }
	VMATH_INLINE friend ElementType MagSqr(ArgType a) { return Dot(a,a); }
	VMATH_INLINE friend ElementType Mag(ArgType a) { return Sqrt(MagSqr(a)); }

	VMATH_INLINE friend OutType Normalize(ArgType a) { return a*RecipSqrt(MagSqr(a)); }

	class BoolV
	{
	public:
		typedef const BoolV& ArgType;

		VMATH_INLINE BoolV(bool x,bool y,bool z,bool w): m_x(x),m_y(y),m_z(z),m_w(w) {}

		VMATH_INLINE friend const BoolV operator ==(ArgType a,ArgType b) { return BoolV(a.x() == b.x(),a.y() == b.y(),a.z() == b.z(),a.w() == b.w()); }
		VMATH_INLINE friend const BoolV operator !=(ArgType a,ArgType b) { return BoolV(a.x() != b.x(),a.y() != b.y(),a.z() != b.z(),a.w() != b.w()); }
		VMATH_INLINE friend const BoolV operator &&(ArgType a,ArgType b) { return BoolV(a.x() && b.x(),a.y() && b.y(),a.z() && b.z(),a.w() && b.w()); }
		VMATH_INLINE friend const BoolV operator ||(ArgType a,ArgType b) { return BoolV(a.x() || b.x(),a.y() || b.y(),a.z() || b.z(),a.w() || b.w()); }
		VMATH_INLINE friend const BoolV operator !(ArgType a) { return BoolV(!a.x(),!a.y(),!a.z(),!a.w()); }

		bool m_x,m_y,m_z,m_w;
	};
	VMATH_INLINE friend const BoolV operator ==(ArgType a,ArgType b) { return BoolV(a.x() == b.x(),a.y() == b.y(),a.z() == b.z(),a.w() == b.w()); }
	VMATH_INLINE friend const BoolV operator !=(ArgType a,ArgType b) { return BoolV(a.x() != b.x(),a.y() != b.y(),a.z() != b.z(),a.w() != b.w()); }
	VMATH_INLINE friend const BoolV operator < (ArgType a,ArgType b) { return BoolV(a.x() <  b.x(),a.y() <  b.y(),a.z() <  b.z(),a.w() <  b.w()); }
	VMATH_INLINE friend const BoolV operator <=(ArgType a,ArgType b) { return BoolV(a.x() <= b.x(),a.y() <= b.y(),a.z() <= b.z(),a.w() <= b.w()); }
	VMATH_INLINE friend const BoolV operator > (ArgType a,ArgType b) { return BoolV(a.x() >  b.x(),a.y() >  b.y(),a.z() >  b.z(),a.w() >  b.w()); }
	VMATH_INLINE friend const BoolV operator >=(ArgType a,ArgType b) { return BoolV(a.x() >= b.x(),a.y() >= b.y(),a.z() >= b.z(),a.w() >= b.w()); }
	
	VMATH_INLINE friend const BoolV operator ==(ArgType a,ElementType b) { return operator ==(a,T(b)); }
	VMATH_INLINE friend const BoolV operator !=(ArgType a,ElementType b) { return operator !=(a,T(b)); }
	VMATH_INLINE friend const BoolV operator < (ArgType a,ElementType b) { return operator < (a,T(b)); }
	VMATH_INLINE friend const BoolV operator <=(ArgType a,ElementType b) { return operator <=(a,T(b)); }
	VMATH_INLINE friend const BoolV operator > (ArgType a,ElementType b) { return operator > (a,T(b)); }
	VMATH_INLINE friend const BoolV operator >=(ArgType a,ElementType b) { return operator >=(a,T(b)); }

	VMATH_INLINE friend const BoolV operator ==(ElementType a,ArgType b) { return operator ==(T(a),b); }
	VMATH_INLINE friend const BoolV operator !=(ElementType a,ArgType b) { return operator !=(T(a),b); }
	VMATH_INLINE friend const BoolV operator < (ElementType a,ArgType b) { return operator < (T(a),b); }
	VMATH_INLINE friend const BoolV operator <=(ElementType a,ArgType b) { return operator <=(T(a),b); }
	VMATH_INLINE friend const BoolV operator > (ElementType a,ArgType b) { return operator > (T(a),b); }
	VMATH_INLINE friend const BoolV operator >=(ElementType a,ArgType b) { return operator >=(T(a),b); }
	
	VMATH_INLINE friend OutType Select(ArgType a,ArgType b,typename BoolV::ArgType sel)
	{
		return T(
			sel.m_x ? b.x() : a.x(),
			sel.m_y ? b.y() : a.y(),
			sel.m_z ? b.z() : a.z(),
			sel.m_w ? b.w() : a.w());
	}

	VMATH_INLINE friend OutType Select0000(ArgType arg0,ArgType arg1) { return arg0; }
	VMATH_INLINE friend OutType Select0001(ArgType arg0,ArgType arg1) { return T(arg0.x(),arg0.y(),arg0.z(),arg1.w()); }
	VMATH_INLINE friend OutType Select0010(ArgType arg0,ArgType arg1) { return T(arg0.x(),arg0.y(),arg1.z(),arg0.w()); }
	VMATH_INLINE friend OutType Select0011(ArgType arg0,ArgType arg1) { return T(arg0.x(),arg0.y(),arg1.z(),arg1.w()); }
	VMATH_INLINE friend OutType Select0100(ArgType arg0,ArgType arg1) { return T(arg0.x(),arg1.y(),arg0.z(),arg0.w()); }
	VMATH_INLINE friend OutType Select0101(ArgType arg0,ArgType arg1) { return T(arg0.x(),arg1.y(),arg0.z(),arg1.w()); }
	VMATH_INLINE friend OutType Select0110(ArgType arg0,ArgType arg1) { return T(arg0.x(),arg1.y(),arg1.z(),arg0.w()); }
	VMATH_INLINE friend OutType Select0111(ArgType arg0,ArgType arg1) { return T(arg0.x(),arg1.y(),arg1.z(),arg1.w()); }
	VMATH_INLINE friend OutType Select1000(ArgType arg0,ArgType arg1) { return T(arg1.x(),arg0.y(),arg0.z(),arg0.w()); }
	VMATH_INLINE friend OutType Select1001(ArgType arg0,ArgType arg1) { return T(arg1.x(),arg0.y(),arg0.z(),arg1.w()); }
	VMATH_INLINE friend OutType Select1010(ArgType arg0,ArgType arg1) { return T(arg1.x(),arg0.y(),arg1.z(),arg0.w()); }
	VMATH_INLINE friend OutType Select1011(ArgType arg0,ArgType arg1) { return T(arg1.x(),arg0.y(),arg1.z(),arg1.w()); }
	VMATH_INLINE friend OutType Select1100(ArgType arg0,ArgType arg1) { return T(arg1.x(),arg1.y(),arg0.z(),arg0.w()); }
	VMATH_INLINE friend OutType Select1101(ArgType arg0,ArgType arg1) { return T(arg1.x(),arg1.y(),arg0.z(),arg1.w()); }
	VMATH_INLINE friend OutType Select1110(ArgType arg0,ArgType arg1) { return T(arg1.x(),arg1.y(),arg1.z(),arg0.w()); }
	VMATH_INLINE friend OutType Select1111(ArgType arg0,ArgType arg1) { return arg1; }

private:
	ElementType m_x,m_y,m_z,m_w;
};

typedef Vec2_T<int> Vec2i;
typedef Vec3_T<int> Vec3i;
typedef Vec4_T<int> Vec4i;
typedef Vec2i::ArgType Vec2i_arg;
typedef Vec3i::ArgType Vec3i_arg;
typedef Vec4i::ArgType Vec4i_arg;
typedef Vec2i::OutType Vec2i_out;
typedef Vec3i::OutType Vec3i_out;
typedef Vec4i::OutType Vec4i_out;

typedef Vec2_T<float> Vec2f;
typedef Vec3_T<float> Vec3f;
typedef Vec4_T<float> Vec4f;
typedef Vec2f::ArgType Vec2f_arg;
typedef Vec3f::ArgType Vec3f_arg;
typedef Vec4f::ArgType Vec4f_arg;
typedef Vec2f::OutType Vec2f_out;
typedef Vec3f::OutType Vec3f_out;
typedef Vec4f::OutType Vec4f_out;

typedef Vec2_T<double> Vec2d;
typedef Vec3_T<double> Vec3d;
typedef Vec4_T<double> Vec4d;
typedef Vec2d::ArgType Vec2d_arg;
typedef Vec3d::ArgType Vec3d_arg;
typedef Vec4d::ArgType Vec4d_arg;
typedef Vec2d::OutType Vec2d_out;
typedef Vec3d::OutType Vec3d_out;
typedef Vec4d::OutType Vec4d_out;

#endif // _INCLUDE_VMATH_FLOATVEC_H_

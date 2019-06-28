// ===========================
// common/vmath/vmath_scalar.h
// ===========================

#ifndef _INCLUDE_VMATH_SCALAR_H_
#define _INCLUDE_VMATH_SCALAR_H_

#include "vmath_common.h"

class ScalarV // scalar value stored as a 4-component SIMD (all four values must be equal)
{
public:
	typedef ScalarV T;
	typedef const T& ArgType;
	typedef const T OutType;
	typedef __m128 IntrinsicType;
	typedef __m128i IntrinsicTypeInt;
	typedef float ElementType;
	typedef uint32 ElementTypeInt;
	enum { NumElements = 1 };

	VMATH_INLINE ScalarV() {}
	VMATH_INLINE explicit ScalarV(IntrinsicType v): m_v(v) {}
	VMATH_INLINE explicit ScalarV(ElementType f): m_v(_mm_set1_ps(f)) {}
	VMATH_INLINE ScalarV(int x): m_v(_mm_cvtepi32_ps(_mm_set1_epi32(x))) {}
	VMATH_INLINE ScalarV(VectorConstantInitializer constant)
	{
		switch (constant) {
		case V_ZERO: m_v = _mm_setzero_ps(); break;
		case V_ONE: m_v = _mm_set1_ps(1.0f); break;
		default: DEBUG_ASSERT(false);
		}
	}

	VMATH_INLINE operator IntrinsicType() const { return m_v; }
	VMATH_INLINE IntrinsicType v() const { return m_v; }
	VMATH_INLINE IntrinsicType& v_ref() { return m_v; }

	VMATH_INLINE float f() const { return _vmath_extract_ps(m_v,0); }

	VMATH_INLINE friend OutType operator +(ArgType a) { return a; } // no-op
	VMATH_INLINE friend OutType operator -(ArgType a) { return T(_vmath_negate_ps(a)); }
	VMATH_INLINE friend OutType operator +(ArgType a,ArgType b) { return T(_mm_add_ps(a,b)); }
	VMATH_INLINE friend OutType operator -(ArgType a,ArgType b) { return T(_mm_sub_ps(a,b)); }
	VMATH_INLINE friend OutType operator *(ArgType a,ArgType b) { return T(_mm_mul_ps(a,b)); }
	VMATH_INLINE friend OutType operator /(ArgType a,ArgType b) { return T(_mm_div_ps(a,b)); }
	VMATH_INLINE friend OutType operator *(ArgType a,ElementType b) { return T(_mm_mul_ps(a,_mm_set1_ps(b))); }
	VMATH_INLINE friend OutType operator /(ArgType a,ElementType b) { return T(_mm_div_ps(a,_mm_set1_ps(b))); }
	VMATH_INLINE friend OutType operator *(ElementType a,ArgType b) { return T(_mm_mul_ps(_mm_set1_ps(a),b)); }
	VMATH_INLINE friend OutType operator /(ElementType a,ArgType b) { return T(_mm_div_ps(_mm_set1_ps(a),b)); }
	VMATH_INLINE friend OutType operator &(ArgType a,ArgType b) { return T(_mm_and_ps(a,b)); }
	VMATH_INLINE friend OutType operator |(ArgType a,ArgType b) { return T(_mm_or_ps(a,b)); }
	VMATH_INLINE friend OutType operator ^(ArgType a,ArgType b) { return T(_mm_xor_ps(a,b)); }
	VMATH_INLINE friend OutType operator &(ArgType a,ElementTypeInt b) { return T(_mm_and_ps(a,_mm_castsi128_ps(_mm_set1_epi32(b)))); }
	VMATH_INLINE friend OutType operator |(ArgType a,ElementTypeInt b) { return T(_mm_or_ps(a,_mm_castsi128_ps(_mm_set1_epi32(b)))); }
	VMATH_INLINE friend OutType operator ^(ArgType a,ElementTypeInt b) { return T(_mm_xor_ps(a,_mm_castsi128_ps(_mm_set1_epi32(b)))); }

	VMATH_INLINE T& operator +=(ArgType b) { m_v = _mm_add_ps(m_v,b); return *this; }
	VMATH_INLINE T& operator -=(ArgType b) { m_v = _mm_sub_ps(m_v,b); return *this; }
	VMATH_INLINE T& operator *=(ArgType b) { m_v = _mm_mul_ps(m_v,b); return *this; }
	VMATH_INLINE T& operator /=(ArgType b) { m_v = _mm_div_ps(m_v,b); return *this; }
	VMATH_INLINE T& operator *=(ElementType b) { m_v = _mm_mul_ps(m_v,_mm_set1_ps(b)); return *this; }
	VMATH_INLINE T& operator /=(ElementType b) { m_v = _mm_div_ps(m_v,_mm_set1_ps(b)); return *this; }
	VMATH_INLINE T& operator &=(ArgType b) { m_v = _mm_and_ps(m_v,b); return *this; }
	VMATH_INLINE T& operator |=(ArgType b) { m_v = _mm_or_ps(m_v,b); return *this; }
	VMATH_INLINE T& operator ^=(ArgType b) { m_v = _mm_xor_ps(m_v,b); return *this; }
	VMATH_INLINE T& operator &=(ElementTypeInt b) { m_v = _mm_and_ps(m_v,_mm_castsi128_ps(_mm_set1_epi32(b))); return *this; }
	VMATH_INLINE T& operator |=(ElementTypeInt b) { m_v = _mm_or_ps(m_v,_mm_castsi128_ps(_mm_set1_epi32(b))); return *this; }
	VMATH_INLINE T& operator ^=(ElementTypeInt b) { m_v = _mm_xor_ps(m_v,_mm_castsi128_ps(_mm_set1_epi32(b))); return *this; }

	VMATH_INLINE friend OutType Min(ArgType a,ArgType b) { return T(_mm_min_ps(a,b)); }
	VMATH_INLINE friend OutType Max(ArgType a,ArgType b) { return T(_mm_max_ps(a,b)); }
	VMATH_INLINE friend OutType Min(ArgType a,ArgType b,ArgType c) { return T(_mm_min_ps(_mm_min_ps(a,b),c)); }
	VMATH_INLINE friend OutType Max(ArgType a,ArgType b,ArgType c) { return T(_mm_max_ps(_mm_max_ps(a,b),c)); }
	VMATH_INLINE friend OutType Min(ArgType a,ArgType b,ArgType c,ArgType d) { return T(_mm_min_ps(_mm_min_ps(a,b),_mm_min_ps(c,d))); }
	VMATH_INLINE friend OutType Max(ArgType a,ArgType b,ArgType c,ArgType d) { return T(_mm_max_ps(_mm_max_ps(a,b),_mm_max_ps(c,d))); }
	VMATH_INLINE friend OutType Abs(ArgType a) { return T(_vmath_abs_ps(a)); }
	VMATH_INLINE friend OutType Clamp(ArgType a,ArgType minVal,ArgType maxVal) { return T(_mm_min_ps(_mm_max_ps(a,minVal),maxVal)); }
	VMATH_INLINE friend OutType Saturate(ArgType a) { return T(_mm_min_ps(_mm_max_ps(a,_mm_setzero_ps()),_mm_set1_ps(1.0f))); }
	VMATH_INLINE friend OutType Floor(ArgType a) { return T(_vmath_floor_ps(a)); } // round down (towards negative infinity)
	VMATH_INLINE friend OutType Ceiling(ArgType a) { return T(_vmath_ceil_ps(a)); } // round up (towards positive infinity)
	VMATH_INLINE friend OutType Truncate(ArgType a) { return T(_vmath_trunc_ps(a)); } // round towards zero
	VMATH_INLINE friend OutType Round(ArgType a) { return T(_vmath_round_ps(a)); } // round to nearest
	VMATH_INLINE friend OutType Frac(ArgType a) { return T(_mm_sub_ps(a,_vmath_floor_ps(a))); }
	VMATH_INLINE friend OutType Sqrt(ArgType a) { return T(_mm_sqrt_ps(a)); }
	VMATH_INLINE friend OutType Sqr(ArgType a) { return T(_mm_mul_ps(a,a)); }
	VMATH_INLINE friend OutType RecipEst(ArgType a) { return T(_mm_rcp_ps(a)); }
	VMATH_INLINE friend OutType Recip(ArgType a) { return T(_vmath_recip_ps(a)); }
	VMATH_INLINE friend OutType RecipSqrtEst(ArgType a) { return T(_mm_rsqrt_ps(a)); }
	VMATH_INLINE friend OutType RecipSqrt(ArgType a) { return T(_vmath_rsqrt_ps(a)); }

	VMATH_INLINE friend OutType MultiplyAdd(ArgType a,ArgType b,ArgType c) { return T(_mm_fmadd_ps(a,b,c)); } // a*b + c
	VMATH_INLINE friend OutType MultiplySub(ArgType a,ArgType b,ArgType c) { return T(_mm_fmsub_ps(a,b,c)); } // a*b - c
	VMATH_INLINE friend OutType NegMultiplyAdd(ArgType a,ArgType b,ArgType c) { return T(_mm_fnmadd_ps(a,b,c)); } // -(a*b - c) = c - a*b
	VMATH_INLINE friend OutType NegMultiplySub(ArgType a,ArgType b,ArgType c) { return T(_mm_fnmsub_ps(a,b,c)); } // -(a*b + c)
	VMATH_INLINE friend OutType MultiplyAdd(ArgType a1,ArgType b1,ArgType a2,ArgType b2) { return MultiplyAdd(a1,b1,a2*b2); } // a1*b1 + a2*b2
	VMATH_INLINE friend OutType MultiplySub(ArgType a1,ArgType b1,ArgType a2,ArgType b2) { return MultiplySub(a1,b1,a2*b2); } // a1*b1 - a2*b2
	VMATH_INLINE friend OutType MultiplyAdd(ArgType a1,ArgType b1,ArgType a2,ArgType b2,ArgType c) { return MultiplyAdd(a1,b1,MultiplyAdd(a2,b2,c)); } // a1*b1 + a2*b2 + c
	VMATH_INLINE friend OutType MultiplyAdd(ArgType a1,ArgType b1,ArgType a2,ArgType b2,ArgType a3,ArgType b3) { return MultiplyAdd(a1,b1,MultiplyAdd(a2,b2,a3*b3)); } // a1*b1 + a2*b2 + a3*b3
	VMATH_INLINE friend OutType MultiplyAdd(ArgType a1,ArgType b1,ArgType a2,ArgType b2,ArgType a3,ArgType b3,ArgType c) { return MultiplyAdd(a1,b1,MultiplyAdd(a2,b2,MultiplyAdd(a3,b3,c))); } // a1*b1 + a2*b2 + a3*b3 + c
	VMATH_INLINE friend OutType MultiplyAdd(ArgType a1,ArgType b1,ArgType a2,ArgType b2,ArgType a3,ArgType b3,ArgType a4,ArgType b4) { return MultiplyAdd(a1,b1,MultiplyAdd(a2,b2,MultiplyAdd(a3,b3,a4*b4))); } // a1*b1 + a2*b2 + a3*b3 + a3*b4

	VMATH_INLINE friend bool IsFinite(ArgType a) { return ::IsFinite(a.f()); }
	VMATH_INLINE friend bool IsNaN(ArgType a) { return ::IsNaN(a.f()); }

	VMATH_INLINE static OutType Load(const float src[NumElements]) { return T(_vmath_broadcast_ss(src)); }
	VMATH_INLINE void Store(float dst[NumElements]) const { _mm_store_ss(dst,m_v); }
#if defined(SN_TARGET_PS4)
	VMATH_INLINE static OutType LoadFloat16(const uint16 src[NumElements]) { return T(_mm_set1_ps(_cvtsh_ss(src[0]))); }
	VMATH_INLINE void StoreFloat16(uint16 dst[NumElements]) const { dst[0] = _cvtss_sh(f(),0); }
#else
	VMATH_INLINE static OutType LoadFloat16(const uint16 src[NumElements]) { const uint32 temp = src[0]; const IntrinsicType v = _mm_cvtph_ps(_mm_castps_si128(_mm_load_ss(reinterpret_cast<const float*>(&temp)))); return T(_vmath_splatx_ps(v)); }
	VMATH_INLINE void StoreFloat16(uint16 dst[NumElements]) const { const uint32 temp = (uint32)_mm_cvtsi128_si32(_mm_cvtps_ph(m_v,0)); dst[0] = (uint16)temp; }
#endif

	VMATH_INLINE friend bool operator ==(ArgType a,ArgType b) { return _mm_movemask_ps(_mm_cmpeq_ps(a,b)) != 0; }
	VMATH_INLINE friend bool operator !=(ArgType a,ArgType b) { return _mm_movemask_ps(_mm_cmpneq_ps(a,b)) != 0; }
	VMATH_INLINE friend bool operator < (ArgType a,ArgType b) { return _mm_movemask_ps(_mm_cmplt_ps(a,b)) != 0; }
	VMATH_INLINE friend bool operator <=(ArgType a,ArgType b) { return _mm_movemask_ps(_mm_cmple_ps(a,b)) != 0; }
	VMATH_INLINE friend bool operator > (ArgType a,ArgType b) { return _mm_movemask_ps(_mm_cmpgt_ps(a,b)) != 0; }
	VMATH_INLINE friend bool operator >=(ArgType a,ArgType b) { return _mm_movemask_ps(_mm_cmpge_ps(a,b)) != 0; }

	VMATH_INLINE friend bool operator ==(ArgType a,ElementType b) { return a.f() == b; }
	VMATH_INLINE friend bool operator !=(ArgType a,ElementType b) { return a.f() != b; }
	VMATH_INLINE friend bool operator < (ArgType a,ElementType b) { return a.f() <  b; }
	VMATH_INLINE friend bool operator <=(ArgType a,ElementType b) { return a.f() <= b; }
	VMATH_INLINE friend bool operator > (ArgType a,ElementType b) { return a.f() >  b; }
	VMATH_INLINE friend bool operator >=(ArgType a,ElementType b) { return a.f() >= b; }

	VMATH_INLINE friend bool operator ==(ElementType a,ArgType b) { return a == b.f(); }
	VMATH_INLINE friend bool operator !=(ElementType a,ArgType b) { return a != b.f(); }
	VMATH_INLINE friend bool operator < (ElementType a,ArgType b) { return a <  b.f(); }
	VMATH_INLINE friend bool operator <=(ElementType a,ArgType b) { return a <= b.f(); }
	VMATH_INLINE friend bool operator > (ElementType a,ArgType b) { return a >  b.f(); }
	VMATH_INLINE friend bool operator >=(ElementType a,ArgType b) { return a >= b.f(); }

private:
	IntrinsicType m_v;
	friend class Vec2V;
	friend class Vec3V;
	friend class Vec4V;
};

template <typename T> VMATH_INLINE uint32 GetBoolMask(T b) { return b.GetMask(); }
template <> VMATH_INLINE uint32 GetBoolMask<bool>(bool b) { return b ? 1 : 0; }
template <> VMATH_INLINE uint32 GetBoolMask<float>(float b) { return *reinterpret_cast<const uint32*>(&b) >> 31; }
template <> VMATH_INLINE uint32 GetBoolMask<ScalarV>(ScalarV b) { return _mm_movemask_ps(b) & 0x0001; }

typedef typename ScalarV::ArgType ScalarV_arg;
typedef typename ScalarV::OutType ScalarV_out;

VMATH_INLINE void PrintV(const char* name,ScalarV_arg v) { printf("%s=%f\n",name,v.f()); }
VMATH_INLINE void PrintV(const char* name,float f) { printf("%s=%f\n",name,f); }

#endif // _INCLUDE_VMATH_SCALAR_H_

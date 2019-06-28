// ==========================
// common/vmath/vmath_vec16.h
// ==========================

#ifndef _INCLUDE_VMATH_VEC16_H_
#define _INCLUDE_VMATH_VEC16_H_

#include "vmath_common.h"
#include "vmath_vec8.h"

#if HAS_VEC16V
class Vec16V
{
public:
	typedef Vec16V T;
	typedef const T& ArgType;
	typedef const T OutType;
	typedef __m512 IntrinsicType;
	typedef float ComponentType;
	typedef float ElementType;
	typedef uint32 ElementTypeInt;
	enum { NumElements = 16 };

	VMATH_INLINE Vec16V() {}
	VMATH_INLINE explicit Vec16V(IntrinsicType v): m_v(v) {}
	VMATH_INLINE explicit Vec16V(IntrinsicTypeInt v): m_v(_mm512_castsi512_ps(v)) {}
	VMATH_INLINE explicit Vec16V(ElementType f): m_v(_mm512_set1_ps(f)) {}
	VMATH_INLINE Vec16V(ScalarV::ArgType s): m_v(_mm512_broadcastss_ps(s)) {}
#if defined(__AVX512DQ__)
	VMATH_INLINE Vec16V(Vec8V::ArgType v01,Vec8V::ArgType v23): m_v(_mm512_insertf32x8(_mm512_castps256_ps512(v01),v23,1)) {}
#else
	VMATH_INLINE Vec16V(Vec8V::ArgType v01,Vec8V::ArgType v23): m_v(_mm512_castpd_ps(_mm512_insertf64x4(_mm512_castps_pd(_mm512_castps256_ps512(v01)),_mm256_castps_pd(v23),1))) {}
#endif
	VMATH_INLINE Vec16V(Vec4V::ArgType v0,Vec4V::ArgType v1,Vec4V::ArgType v2,Vec4V::ArgType v3): m_v(_mm512_insertf128_ps(_mm512_insertf128_ps(_mm512_insertf128_ps(_mm512_castps128_ps512(v0),v1,1),v2,2),v3,3)) {}
	VMATH_INLINE Vec16V(
		ElementType x0,ElementType y0,ElementType z0,ElementType w0,
		ElementType x1,ElementType y1,ElementType z1,ElementType w1,
		ElementType x2,ElementType y2,ElementType z2,ElementType w2,
		ElementType x3,ElementType y3,ElementType z3,ElementType w3): m_v(_mm512_setr_ps(x0,y0,z0,w0,x1,y1,z1,w1,x2,y2,z2,w2,x3,y3,z3,w3)) {}
	VMATH_INLINE Vec16V(
		int x0,int y0,int z0,int w0,
		int x1,int y1,int z1,int w1,
		int x2,int y2,int z2,int w2,
		int x3,int y3,int z3,int w3): m_v(_mm512_cvtepi32_ps(_mm512_setr_epi32(x0,y0,z0,w0,x1,y1,z1,w1,x2,y2,z2,w2,x3,y3,z3,w3))) {}
	VMATH_INLINE Vec16V(
		uint32 x0,uint32 y0,uint32 z0,uint32 w0,
		uint32 x1,uint32 y1,uint32 z1,uint32 w1,
		uint32 x2,uint32 y2,uint32 z2,uint32 w2,
		uint32 x3,uint32 y3,uint32 z3,uint32 w3): m_v(_mm512_cvtepi32_ps(_mm512_setr_epi32(x0,y0,z0,w0,x1,y1,z1,w1,x2,y2,z2,w2,x3,y3,z3,w3))) {}
	VMATH_INLINE Vec16V(VectorConstantInitializer constant)
	{
		switch (constant) {
		case V_ZERO: m_v = _mm512_setzero_ps(); break;
		case V_ONE: m_v = _mm512_set1_ps(1.0f); break;
		default: DEBUG_ASSERT(false);
		}
	}

	VMATH_INLINE operator IntrinsicType() const { return m_v; }
	VMATH_INLINE IntrinsicType v() const { return m_v; }
	VMATH_INLINE IntrinsicType& v_ref() { return m_v; }

	VMATH_INLINE Vec4V_out v0() const { return Vec4V(_mm512_castps512_ps128(m_v)); }
	VMATH_INLINE Vec4V_out v1() const { return Vec4V(_mm512_extractf32x4_ps(m_v,1)); }
	VMATH_INLINE Vec4V_out v2() const { return Vec4V(_mm512_extractf32x4_ps(m_v,2)); }
	VMATH_INLINE Vec4V_out v3() const { return Vec4V(_mm512_extractf32x4_ps(m_v,3)); }

	VMATH_INLINE Vec8V_out v01() const { return Vec8V(_mm512_castps512_ps256(m_v)); }
	VMATH_INLINE Vec8V_out v23() const { return Vec8V(_mm512_extractf32x8_ps(m_v,1)); }

	VMATH_INLINE ElementType operator [](unsigned index) const { DEBUG_ASSERT(index < NumElements); return m_v.m512_f32[index]; }
	VMATH_INLINE ElementType& operator [](unsigned index) { DEBUG_ASSERT(index < NumElements); return m_v.m512_f32[index]; }

	VMATH_INLINE friend OutType operator +(ArgType a) { return a; } // no-op
	VMATH_INLINE friend OutType operator -(ArgType a) { return T(_mm512_sub_ps(_mm512_setzero_ps(),a)); }
	VMATH_INLINE friend OutType operator +(ArgType a,ArgType b) { return T(_mm512_add_ps(a,b)); }
	VMATH_INLINE friend OutType operator -(ArgType a,ArgType b) { return T(_mm512_sub_ps(a,b)); }
	VMATH_INLINE friend OutType operator *(ArgType a,ArgType b) { return T(_mm512_mul_ps(a,b)); }
	VMATH_INLINE friend OutType operator /(ArgType a,ArgType b) { return T(_mm512_div_ps(a,b)); }
	VMATH_INLINE friend OutType operator *(ArgType a,ElementType b) { return T(_mm512_mul_ps(a,_mm512_set1_ps(b))); }
	VMATH_INLINE friend OutType operator /(ArgType a,ElementType b) { return T(_mm512_div_ps(a,_mm512_set1_ps(b))); }
	VMATH_INLINE friend OutType operator *(ElementType a,ArgType b) { return T(_mm512_mul_ps(_mm512_set1_ps(a),b)); }
	VMATH_INLINE friend OutType operator /(ElementType a,ArgType b) { return T(_mm512_div_ps(_mm512_set1_ps(a),b)); }
	VMATH_INLINE friend OutType operator &(ArgType a,ArgType b) { return T(_mm512_and_ps(a,b)); }
	VMATH_INLINE friend OutType operator |(ArgType a,ArgType b) { return T(_mm512_or_ps(a,b)); }
	VMATH_INLINE friend OutType operator ^(ArgType a,ArgType b) { return T(_mm512_xor_ps(a,b)); }
	VMATH_INLINE friend OutType operator &(ArgType a,ElementTypeInt b) { return T(_mm512_and_ps(a,_mm512_castsi512_ps(_mm512_set1_epi32(b)))); }
	VMATH_INLINE friend OutType operator |(ArgType a,ElementTypeInt b) { return T(_mm512_or_ps(a,_mm512_castsi512_ps(_mm512_set1_epi32(b)))); }
	VMATH_INLINE friend OutType operator ^(ArgType a,ElementTypeInt b) { return T(_mm512_xor_ps(a,_mm512_castsi512_ps(_mm512_set1_epi32(b)))); }

	VMATH_INLINE T& operator +=(ArgType b) { m_v = _mm512_add_ps(m_v,b); return *this; }
	VMATH_INLINE T& operator -=(ArgType b) { m_v = _mm512_sub_ps(m_v,b); return *this; }
	VMATH_INLINE T& operator *=(ArgType b) { m_v = _mm512_mul_ps(m_v,b); return *this; }
	VMATH_INLINE T& operator /=(ArgType b) { m_v = _mm512_div_ps(m_v,b); return *this; }
	VMATH_INLINE T& operator *=(ElementType b) { m_v = _mm512_mul_ps(m_v,_mm512_set1_ps(b)); return *this; }
	VMATH_INLINE T& operator /=(ElementType b) { m_v = _mm512_div_ps(m_v,_mm512_set1_ps(b)); return *this; }
	VMATH_INLINE T& operator &=(ArgType b) { m_v = _mm512_and_ps(m_v,b); return *this; }
	VMATH_INLINE T& operator |=(ArgType b) { m_v = _mm512_or_ps(m_v,b); return *this; }
	VMATH_INLINE T& operator ^=(ArgType b) { m_v = _mm512_xor_ps(m_v,b); return *this; }
	VMATH_INLINE T& operator &=(ElementTypeInt b) { m_v = _mm512_and_ps(m_v,_mm512_castsi256_ps(_mm512_set1_epi32(b))); return *this; }
	VMATH_INLINE T& operator |=(ElementTypeInt b) { m_v = _mm512_or_ps(m_v,_mm512_castsi256_ps(_mm512_set1_epi32(b))); return *this; }
	VMATH_INLINE T& operator ^=(ElementTypeInt b) { m_v = _mm512_xor_ps(m_v,_mm512_castsi256_ps(_mm512_set1_epi32(b))); return *this; }

	VMATH_INLINE friend OutType Min(ArgType a,ArgType b) { return T(_mm512_min_ps(a,b)); }
	VMATH_INLINE friend OutType Max(ArgType a,ArgType b) { return T(_mm512_max_ps(a,b)); }
	VMATH_INLINE friend OutType Min(ArgType a,ArgType b,ArgType c) { return T(_mm512_min_ps(_mm512_min_ps(a,b),c)); }
	VMATH_INLINE friend OutType Max(ArgType a,ArgType b,ArgType c) { return T(_mm512_max_ps(_mm512_max_ps(a,b),c)); }
	VMATH_INLINE friend OutType Min(ArgType a,ArgType b,ArgType c,ArgType d) { return T(_mm512_min_ps(_mm512_min_ps(a,b),_mm512_min_ps(c,d))); }
	VMATH_INLINE friend OutType Max(ArgType a,ArgType b,ArgType c,ArgType d) { return T(_mm512_max_ps(_mm512_max_ps(a,b),_mm512_max_ps(c,d))); }
	VMATH_INLINE friend OutType Abs(ArgType a) { return T(_mm512_abs_ps(a)); } // _mm512_andnot_ps(_mm512_castsi512_ps(_mm512_set1_epi32(0x80000000)),a)); }
	VMATH_INLINE friend OutType Clamp(ArgType a,ArgType minVal,ArgType maxVal) { return T(_mm512_min_ps(_mm512_max_ps(a,minVal),maxVal)); }
	VMATH_INLINE friend OutType Saturate(ArgType a) { return T(_mm512_min_ps(_mm512_max_ps(a,_mm512_setzero_ps()),_mm512_set1_ps(1.0f))); }
	VMATH_INLINE friend OutType Floor(ArgType a) { return T(_mm_round_ps(a,_MM_FROUND_TO_NEG_INF,_MM_EXPADJ_NONE)); } // round down (towards negative infinity)
	VMATH_INLINE friend OutType Ceiling(ArgType a) { return T(_mm_round_ps(a,_MM_FROUND_TO_POS_INF,_MM_EXPADJ_NONE)); } // round up (towards positive infinity)
	VMATH_INLINE friend OutType Truncate(ArgType a) { return T(_mm_round_ps(a,_MM_FROUND_TO_ZERO,_MM_EXPADJ_NONE)); } // round towards zero
	VMATH_INLINE friend OutType Round(ArgType a) { return T(_mm_round_ps(a,_MM_FROUND_TO_NEAREST_INT,_MM_EXPADJ_NONE)); } // round to nearest
	VMATH_INLINE friend OutType Frac(ArgType a) { return T(_mm512_sub_ps(a,_mm512_round_ps(a,_MM_FROUND_TO_NEG_INF,_MM_EXPADJ_NONE))); }
	VMATH_INLINE friend OutType Sqrt(ArgType a) { return T(_mm512_sqrt_ps(a)); }
	VMATH_INLINE friend OutType Sqr(ArgType a) { return T(_mm512_mul_ps(a,a)); }
	VMATH_INLINE friend OutType RecipEst(ArgType a) { return T(_mm512_rcp14_ps(a)); }
	VMATH_INLINE friend OutType Recip(ArgType a)
	{
	#if VMATH_NEWTON_RAPHSON_RECIPROCAL_OPTIMIZATION
	#if defined(__AVX512ER__)
		return T(_mm512_rcp28_ps(a));
	#else
		const IntrinsicType two = _mm512_set1_ps(2.0f);
		IntrinsicType est = _mm512_rcp14_ps(a);
		est = _mm512_mul_ps(est,_mm512_fnmadd_ps(a,est,two)); // est <- est*(2 - a*est)
	#endif
		return T(est);
	#else
		return T(_mm512_div_ps(_mm512_set1_ps(1.0f),a));
	#endif
	}
	VMATH_INLINE friend OutType RecipSqrtEst(ArgType a) { return T(_mm512_rsqrt14_ps(a)); }
	VMATH_INLINE friend OutType RecipSqrt(ArgType a)
	{
	#if VMATH_NEWTON_RAPHSON_RECIPROCAL_OPTIMIZATION
		// NOTE -- Embree code uses rsqrt14 + Newton-Raphson if defined(__AVX512VL__)
	#if defined(__AVX512ER__)
		return T(_mm512_rsqrt28_ps(a));
	#else
		const IntrinsicType half = _mm512_set1_ps(0.5f);
		const IntrinsicType three = _mm512_set1_ps(3.0f);
		IntrinsicType est = _mm512_rsqrt14_ps(a);
		est = _mm512_mul_ps(_mm512_mul_ps(est,half),_mm512_fnmadd_ps(a,_mm512_mul_ps(est,est),three)); // est <- est*(3 - a*est^2)/2
		return T(est);
	#endif
	#else
		return T(_mm512_div_ps(_mm512_set1_ps(1.0f),_mm512_sqrt_ps(a)));
	#endif
	}

	VMATH_INLINE friend OutType MultiplyAdd(ArgType a,ArgType b,ArgType c) { return T(_mm512_fmadd_ps(a,b,c)); } // a*b + c
	VMATH_INLINE friend OutType MultiplySub(ArgType a,ArgType b,ArgType c) { return T(_mm512_fmsub_ps(a,b,c)); } // a*b - c
	VMATH_INLINE friend OutType NegMultiplyAdd(ArgType a,ArgType b,ArgType c) { return T(_mm512_fnmadd_ps(a,b,c)); } // -(a*b - c) = c - a*b
	VMATH_INLINE friend OutType NegMultiplySub(ArgType a,ArgType b,ArgType c) { return T(_mm512_fnmsub_ps(a,b,c)); } // -(a*b + c)
	VMATH_INLINE friend OutType MultiplyAdd(ArgType a1,ArgType b1,ArgType a2,ArgType b2) { return MultiplyAdd(a1,b1,a2*b2); } // a1*b1 + a2*b2
	VMATH_INLINE friend OutType MultiplySub(ArgType a1,ArgType b1,ArgType a2,ArgType b2) { return MultiplySub(a1,b1,a2*b2); } // a1*b1 - a2*b2
	VMATH_INLINE friend OutType MultiplyAdd(ArgType a1,ArgType b1,ArgType a2,ArgType b2,ArgType c) { return MultiplyAdd(a1,b1,MultiplyAdd(a2,b2,c)); } // a1*b1 + a2*b2 + c
	VMATH_INLINE friend OutType MultiplyAdd(ArgType a1,ArgType b1,ArgType a2,ArgType b2,ArgType a3,ArgType b3) { return MultiplyAdd(a1,b1,MultiplyAdd(a2,b2,a3*b3)); } // a1*b1 + a2*b2 + a3*b3
	VMATH_INLINE friend OutType MultiplyAdd(ArgType a1,ArgType b1,ArgType a2,ArgType b2,ArgType a3,ArgType b3,ArgType c) { return MultiplyAdd(a1,b1,MultiplyAdd(a2,b2,MultiplyAdd(a3,b3,c))); } // a1*b1 + a2*b2 + a3*b3 + c
	VMATH_INLINE friend OutType MultiplyAdd(ArgType a1,ArgType b1,ArgType a2,ArgType b2,ArgType a3,ArgType b3,ArgType a4,ArgType b4) { return MultiplyAdd(a1,b1,MultiplyAdd(a2,b2,MultiplyAdd(a3,b3,a4*b4))); } // a1*b1 + a2*b2 + a3*b3 + a3*b4

	VMATH_INLINE static OutType LoadScalar(const float* src) { return T(_mm512_broadcastss_ps(_mm_load_ss(src))); } // is there a better way to do this?
	VMATH_INLINE static OutType Load(const T* src) { return T(_mm512_load_ps(reinterpret_cast<const float*>(src))); }
	VMATH_INLINE static OutType LoadUnaligned(const T* src) { return T(_mm512_loadu_ps(reinterpret_cast<const float*>(src))); }
	VMATH_INLINE static OutType LoadStreaming(const T* src) { return T(_mm512_castsi512_ps(_mm512_stream_load_si512(src))); }
	VMATH_INLINE void Store(T* dst) const { _mm512_store_ps(dst,m_v); }
	VMATH_INLINE void StoreUnaligned(T* dst) const { _mm512_storeu_ps(dst,m_v); }
	VMATH_INLINE void StoreStreaming(T* dst) const { _mm512_stream_ps(dst,m_v); }

private:
	IntrinsicType m_v;
};

typedef Vec16V::ArgType Vec16V_arg;
typedef Vec16V::OutType Vec16V_out;

#define VEC16V_ARGS(v) VEC4V_ARGS(v.v0()),VEC4V_ARGS(v.v1()),VEC4V_ARGS(v.v2()),VEC4V_ARGS(v.v3())

VMATH_INLINE void PrintV(const char* name,Vec16V_arg v) { printf("%s=%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n",name,VEC16V_ARGS(v)); }
VMATH_INLINE void PrintV(const char* name,typename Vec16V::IntrinsicType v) { PrintV(name,Vec16V(v)); }
#endif // HAS_VEC16V

#endif // _INCLUDE_VMATH_VEC16_H_

// =========================
// common/vmath/vmath_vec8.h
// =========================

#ifndef _INCLUDE_VMATH_VEC8_H_
#define _INCLUDE_VMATH_VEC8_H_

#include "vmath_common.h"
#include "vmath_vec4.h"

#if HAS_VEC8V
class Vec8V
{
public:
	typedef Vec8V T;
	typedef const T& ArgType;
	typedef const T OutType;
	typedef __m256 IntrinsicType;
	typedef __m256i IntrinsicTypeInt;
	typedef float ComponentType;
	typedef float ElementType;
	typedef uint32 ElementTypeInt;
	enum { NumElements = 8 };

	VMATH_INLINE Vec8V() {}
	VMATH_INLINE explicit Vec8V(IntrinsicType v): m_v(v) {}
	VMATH_INLINE explicit Vec8V(IntrinsicTypeInt v): m_v(_mm256_castsi256_ps(v)) {}
	VMATH_INLINE explicit Vec8V(ElementType f): m_v(_mm256_set1_ps(f)) {}
	VMATH_INLINE Vec8V(ScalarV::ArgType s): m_v(_vmath256_splatx_128_ps(s)) {}
	VMATH_INLINE Vec8V(Vec4V::ArgType v0,Vec4V::ArgType v1): m_v(_mm256_insertf128_ps(_mm256_castps128_ps256(v0),v1,1)) {}
	VMATH_INLINE Vec8V(ElementType x0,ElementType y0,ElementType z0,ElementType w0,ElementType x1,ElementType y1,ElementType z1,ElementType w1): m_v(_mm256_setr_ps(x0,y0,z0,w0,x1,y1,z1,w1)) {}
	VMATH_INLINE Vec8V(int x0,int y0,int z0,int w0,int x1,int y1,int z1,int w1): m_v(_mm256_cvtepi32_ps(_mm256_setr_epi32(x0,y0,z0,w0,x1,y1,z1,w1))) {}
	VMATH_INLINE Vec8V(uint32 x0,uint32 y0,uint32 z0,uint32 w0,uint32 x1,uint32 y1,uint32 z1,uint32 w1): m_v(_mm256_cvtepi32_ps(_mm256_setr_epi32(x0,y0,z0,w0,x1,y1,z1,w1))) {}
	VMATH_INLINE Vec8V(VectorConstantInitializer constant)
	{
		switch (constant) {
		case V_ZERO: m_v = _mm256_setzero_ps(); break;
		case V_ONE: m_v = _mm256_set1_ps(1.0f); break;
		default: DEBUG_ASSERT(false);
		}
	}

	VMATH_INLINE operator IntrinsicType() const { return m_v; }
	VMATH_INLINE IntrinsicType v() const { return m_v; }
	VMATH_INLINE IntrinsicType& v_ref() { return m_v; }

	VMATH_INLINE Vec4V_out v0() const { return Vec4V(_mm256_castps256_ps128(m_v)); }
	VMATH_INLINE Vec4V_out v1() const { return Vec4V(_mm256_extractf128_ps(m_v,1)); }

	VMATH_INLINE ElementType operator [](unsigned index) const { DEBUG_ASSERT(index < NumElements); return _vmath256_extract_ps(m_v,index); }
	VMATH_INLINE ElementType& operator [](unsigned index) { DEBUG_ASSERT(index < NumElements); return _vmath256_ref_ps(m_v,index); }

	VMATH_INLINE friend OutType operator +(ArgType a) { return a; } // no-op
	VMATH_INLINE friend OutType operator -(ArgType a) { return T(_mm256_sub_ps(_mm256_setzero_ps(),a)); }
	VMATH_INLINE friend OutType operator +(ArgType a,ArgType b) { return T(_mm256_add_ps(a,b)); }
	VMATH_INLINE friend OutType operator -(ArgType a,ArgType b) { return T(_mm256_sub_ps(a,b)); }
	VMATH_INLINE friend OutType operator *(ArgType a,ArgType b) { return T(_mm256_mul_ps(a,b)); }
	VMATH_INLINE friend OutType operator /(ArgType a,ArgType b) { return T(_mm256_div_ps(a,b)); }
	VMATH_INLINE friend OutType operator *(ArgType a,ElementType b) { return T(_mm256_mul_ps(a,_mm256_set1_ps(b))); }
	VMATH_INLINE friend OutType operator /(ArgType a,ElementType b) { return T(_mm256_div_ps(a,_mm256_set1_ps(b))); }
	VMATH_INLINE friend OutType operator *(ElementType a,ArgType b) { return T(_mm256_mul_ps(_mm256_set1_ps(a),b)); }
	VMATH_INLINE friend OutType operator /(ElementType a,ArgType b) { return T(_mm256_div_ps(_mm256_set1_ps(a),b)); }
	VMATH_INLINE friend OutType operator &(ArgType a,ArgType b) { return T(_mm256_and_ps(a,b)); }
	VMATH_INLINE friend OutType operator |(ArgType a,ArgType b) { return T(_mm256_or_ps(a,b)); }
	VMATH_INLINE friend OutType operator ^(ArgType a,ArgType b) { return T(_mm256_xor_ps(a,b)); }
	VMATH_INLINE friend OutType operator &(ArgType a,ElementTypeInt b) { return T(_mm256_and_ps(a,_mm256_castsi256_ps(_mm256_set1_epi32(b)))); }
	VMATH_INLINE friend OutType operator |(ArgType a,ElementTypeInt b) { return T(_mm256_or_ps(a,_mm256_castsi256_ps(_mm256_set1_epi32(b)))); }
	VMATH_INLINE friend OutType operator ^(ArgType a,ElementTypeInt b) { return T(_mm256_xor_ps(a,_mm256_castsi256_ps(_mm256_set1_epi32(b)))); }

	VMATH_INLINE T& operator +=(ArgType b) { m_v = _mm256_add_ps(m_v,b); return *this; }
	VMATH_INLINE T& operator -=(ArgType b) { m_v = _mm256_sub_ps(m_v,b); return *this; }
	VMATH_INLINE T& operator *=(ArgType b) { m_v = _mm256_mul_ps(m_v,b); return *this; }
	VMATH_INLINE T& operator /=(ArgType b) { m_v = _mm256_div_ps(m_v,b); return *this; }
	VMATH_INLINE T& operator *=(ElementType b) { m_v = _mm256_mul_ps(m_v,_mm256_set1_ps(b)); return *this; }
	VMATH_INLINE T& operator /=(ElementType b) { m_v = _mm256_div_ps(m_v,_mm256_set1_ps(b)); return *this; }
	VMATH_INLINE T& operator &=(ArgType b) { m_v = _mm256_and_ps(m_v,b); return *this; }
	VMATH_INLINE T& operator |=(ArgType b) { m_v = _mm256_or_ps(m_v,b); return *this; }
	VMATH_INLINE T& operator ^=(ArgType b) { m_v = _mm256_xor_ps(m_v,b); return *this; }
	VMATH_INLINE T& operator &=(ElementTypeInt b) { m_v = _mm256_and_ps(m_v,_mm256_castsi256_ps(_mm256_set1_epi32(b))); return *this; }
	VMATH_INLINE T& operator |=(ElementTypeInt b) { m_v = _mm256_or_ps(m_v,_mm256_castsi256_ps(_mm256_set1_epi32(b))); return *this; }
	VMATH_INLINE T& operator ^=(ElementTypeInt b) { m_v = _mm256_xor_ps(m_v,_mm256_castsi256_ps(_mm256_set1_epi32(b))); return *this; }

	VMATH_INLINE friend OutType Min(ArgType a,ArgType b) { return T(_mm256_min_ps(a,b)); }
	VMATH_INLINE friend OutType Max(ArgType a,ArgType b) { return T(_mm256_max_ps(a,b)); }
	VMATH_INLINE friend OutType Min(ArgType a,ArgType b,ArgType c) { return T(_mm256_min_ps(_mm256_min_ps(a,b),c)); }
	VMATH_INLINE friend OutType Max(ArgType a,ArgType b,ArgType c) { return T(_mm256_max_ps(_mm256_max_ps(a,b),c)); }
	VMATH_INLINE friend OutType Min(ArgType a,ArgType b,ArgType c,ArgType d) { return T(_mm256_min_ps(_mm256_min_ps(a,b),_mm256_min_ps(c,d))); }
	VMATH_INLINE friend OutType Max(ArgType a,ArgType b,ArgType c,ArgType d) { return T(_mm256_max_ps(_mm256_max_ps(a,b),_mm256_max_ps(c,d))); }
	VMATH_INLINE friend OutType Abs(ArgType a) { return T(_mm256_andnot_ps(_mm256_castsi256_ps(_mm256_set1_epi32(0x80000000)),a)); }
	VMATH_INLINE friend OutType Clamp(ArgType a,ArgType minVal,ArgType maxVal) { return T(_mm256_min_ps(_mm256_max_ps(a,minVal),maxVal)); }
	VMATH_INLINE friend OutType Saturate(ArgType a) { return T(_mm256_min_ps(_mm256_max_ps(a,_mm256_setzero_ps()),_mm256_set1_ps(1.0f))); }
	VMATH_INLINE friend OutType Floor(ArgType a) { return T(_mm256_round_ps(a,_MM_FROUND_TO_NEG_INF|_MM_FROUND_NO_EXC)); } // round down (towards negative infinity)
	VMATH_INLINE friend OutType Ceiling(ArgType a) { return T(_mm256_round_ps(a,_MM_FROUND_TO_POS_INF|_MM_FROUND_NO_EXC)); } // round up (towards positive infinity)
	VMATH_INLINE friend OutType Truncate(ArgType a) { return T(_mm256_round_ps(a,_MM_FROUND_TO_ZERO|_MM_FROUND_NO_EXC)); } // round towards zero
	VMATH_INLINE friend OutType Round(ArgType a) { return T(_mm256_round_ps(a,_MM_FROUND_TO_NEAREST_INT|_MM_FROUND_NO_EXC)); } // round to nearest
	VMATH_INLINE friend OutType Frac(ArgType a) { return T(_mm256_sub_ps(a,_mm256_round_ps(a,_MM_FROUND_TO_NEG_INF|_MM_FROUND_NO_EXC))); }
	VMATH_INLINE friend OutType Sqrt(ArgType a) { return T(_mm256_sqrt_ps(a)); }
	VMATH_INLINE friend OutType Sqr(ArgType a) { return T(_mm256_mul_ps(a,a)); }
	VMATH_INLINE friend OutType RecipEst(ArgType a) { return T(_mm256_rcp_ps(a)); }
	VMATH_INLINE friend OutType Recip(ArgType a)
	{
	#if VMATH_NEWTON_RAPHSON_RECIPROCAL_OPTIMIZATION
		const IntrinsicType two = _mm256_set1_ps(2.0f);
	#if defined(__AVX512VL__)
		IntrinsicType est = _mm256_rcp14_ps(a);
	#else
		IntrinsicType est = _mm256_rcp_ps(a);
	#endif
		est = _mm256_mul_ps(est,_mm256_fnmadd_ps(a,est,two)); // est <- est*(2 - a*est)
		return T(est);
	#else
		return T(_mm256_div_ps(_mm256_set1_ps(1.0f),a));
	#endif
	}
	VMATH_INLINE friend OutType RecipSqrtEst(ArgType a) { return T(_mm256_rsqrt_ps(a)); }
	VMATH_INLINE friend OutType RecipSqrt(ArgType a)
	{
	#if VMATH_NEWTON_RAPHSON_RECIPROCAL_OPTIMIZATION
		const IntrinsicType half = _mm256_set1_ps(0.5f);
		const IntrinsicType three = _mm256_set1_ps(3.0f);
	#if defined(__AVX512VL__)
		IntrinsicType est = _mm256_rsqrt14_ps(a);
	#else
		IntrinsicType est = _mm256_rsqrt_ps(a);
	#endif
		est = _mm256_mul_ps(_mm256_mul_ps(est,half),_mm256_fnmadd_ps(a,_mm256_mul_ps(est,est),three)); // est <- est*(3 - a*est^2)/2
		return T(est);
	#else
		return T(_mm256_div_ps(_mm256_set1_ps(1.0f),_mm256_sqrt_ps(a)));
	#endif
	}

	VMATH_INLINE friend OutType MultiplyAdd(ArgType a,ArgType b,ArgType c) { return T(_mm256_fmadd_ps(a,b,c)); } // a*b + c
	VMATH_INLINE friend OutType MultiplySub(ArgType a,ArgType b,ArgType c) { return T(_mm256_fmsub_ps(a,b,c)); } // a*b - c
	VMATH_INLINE friend OutType NegMultiplyAdd(ArgType a,ArgType b,ArgType c) { return T(_mm256_fnmadd_ps(a,b,c)); } // -(a*b - c) = c - a*b
	VMATH_INLINE friend OutType NegMultiplySub(ArgType a,ArgType b,ArgType c) { return T(_mm256_fnmsub_ps(a,b,c)); } // -(a*b + c)
	VMATH_INLINE friend OutType MultiplyAdd(ArgType a1,ArgType b1,ArgType a2,ArgType b2) { return MultiplyAdd(a1,b1,a2*b2); } // a1*b1 + a2*b2
	VMATH_INLINE friend OutType MultiplySub(ArgType a1,ArgType b1,ArgType a2,ArgType b2) { return MultiplySub(a1,b1,a2*b2); } // a1*b1 - a2*b2
	VMATH_INLINE friend OutType MultiplyAdd(ArgType a1,ArgType b1,ArgType a2,ArgType b2,ArgType c) { return MultiplyAdd(a1,b1,MultiplyAdd(a2,b2,c)); } // a1*b1 + a2*b2 + c
	VMATH_INLINE friend OutType MultiplyAdd(ArgType a1,ArgType b1,ArgType a2,ArgType b2,ArgType a3,ArgType b3) { return MultiplyAdd(a1,b1,MultiplyAdd(a2,b2,a3*b3)); } // a1*b1 + a2*b2 + a3*b3
	VMATH_INLINE friend OutType MultiplyAdd(ArgType a1,ArgType b1,ArgType a2,ArgType b2,ArgType a3,ArgType b3,ArgType c) { return MultiplyAdd(a1,b1,MultiplyAdd(a2,b2,MultiplyAdd(a3,b3,c))); } // a1*b1 + a2*b2 + a3*b3 + c
	VMATH_INLINE friend OutType MultiplyAdd(ArgType a1,ArgType b1,ArgType a2,ArgType b2,ArgType a3,ArgType b3,ArgType a4,ArgType b4) { return MultiplyAdd(a1,b1,MultiplyAdd(a2,b2,MultiplyAdd(a3,b3,a4*b4))); } // a1*b1 + a2*b2 + a3*b3 + a3*b4

	VMATH_INLINE friend ScalarV_out MinElement(ArgType a)
	{
		IntrinsicType temp;
		temp = _mm256_min_ps(a,_mm256_permute_ps(a,VMATH_PERMUTE(2,3,0,1)));
		temp = _mm256_min_ps(temp,_mm256_permute_ps(temp,VMATH_PERMUTE(1,0,3,2)));
		temp = _mm256_min_ps(temp,_mm256_permute2f128_ps(temp,temp,1));
		return ScalarV(_mm256_castps256_ps128(temp));
	}

	VMATH_INLINE friend ScalarV_out MaxElement(ArgType a)
	{
		IntrinsicType temp;
		temp = _mm256_max_ps(a,_mm256_permute_ps(a,VMATH_PERMUTE(2,3,0,1)));
		temp = _mm256_max_ps(temp,_mm256_permute_ps(temp,VMATH_PERMUTE(1,0,3,2)));
		temp = _mm256_max_ps(temp,_mm256_permute2f128_ps(temp,temp,1));
		return ScalarV(_mm256_castps256_ps128(temp));
	}

	VMATH_INLINE static OutType LoadScalar(const float* src) { return T(_vmath256_broadcast_ss(src)); }
	VMATH_INLINE static OutType Load(const T* src) { return T(_mm256_load_ps(reinterpret_cast<const float*>(src))); }
	VMATH_INLINE static OutType LoadUnaligned(const T* src) { return T(_mm256_loadu_ps(reinterpret_cast<const float*>(src))); }
	VMATH_INLINE static OutType LoadStreaming(const T* src) { return T(_mm256_castsi256_ps(_mm256_stream_load_si256(reinterpret_cast<const IntrinsicTypeInt*>(src)))); }
	VMATH_INLINE void Store(T* dst) const { _mm256_store_ps(reinterpret_cast<float*>(dst),m_v); }
	VMATH_INLINE void StoreUnaligned(T* dst) const { _mm256_storeu_ps(reinterpret_cast<float*>(dst),m_v); }
	VMATH_INLINE void StoreStreaming(T* dst) const { _mm256_stream_ps(reinterpret_cast<float*>(dst),m_v); }

	VMATH_INLINE friend bool IsFinite(ArgType a)
	{
		const IntrinsicTypeInt mask = _mm256_set1_epi32(0x7F800000);
		return All(BoolV(_mm256_cmpeq_epi32(_mm256_and_si256(_mm256_castps_si256(a),mask),mask)));
	}

	VMATH_INLINE friend bool AnyNaN(ArgType a)
	{
		return NotAll(BoolV(_mm256_cmp_ps(a,a,_CMP_EQ_OQ)));
	}

	class BoolV
	{
	public:
		static const uint32 MaskAll = (1 << NumElements) - 1;
		typedef const BoolV& ArgType;

		VMATH_INLINE explicit BoolV(IntrinsicType v): m_v(v) {}
		VMATH_INLINE explicit BoolV(IntrinsicTypeInt v): m_v(_mm256_castsi256_ps(v)) {}
		VMATH_INLINE operator IntrinsicType() const { return m_v; }
		VMATH_INLINE uint32 GetMask() const { return _mm256_movemask_ps(m_v); }
		VMATH_INLINE friend bool All(ArgType a) { return _mm256_movemask_ps(a) == Vec8V::BoolV::MaskAll; }
		VMATH_INLINE friend bool Any(ArgType a) { return _mm256_movemask_ps(a) != 0; }
		VMATH_INLINE friend bool None(ArgType a) { return _mm256_movemask_ps(a) == 0; }
		VMATH_INLINE friend bool NotAll(ArgType a) { return _mm256_movemask_ps(a) != Vec8V::BoolV::MaskAll; }
		VMATH_INLINE static BoolV AllTrue() { return BoolV(_mm256_set1_epi32(-1)); }
		VMATH_INLINE static BoolV AllFalse() { return BoolV(_mm256_setzero_ps()); }

		VMATH_INLINE friend const BoolV operator ==(ArgType a,ArgType b) { return BoolV(_mm256_xor_ps(_mm256_xor_ps(a,b),_mm256_castsi256_ps(_mm256_set1_epi32(-1)))); } // yuck!
		VMATH_INLINE friend const BoolV operator !=(ArgType a,ArgType b) { return BoolV(_mm256_xor_ps(a,b)); }
		VMATH_INLINE friend const BoolV operator &&(ArgType a,ArgType b) { return BoolV(_mm256_and_ps(a,b)); }
		VMATH_INLINE friend const BoolV operator ||(ArgType a,ArgType b) { return BoolV(_mm256_or_ps(a,b)); }
		VMATH_INLINE friend const BoolV operator !(ArgType a) { return BoolV(_mm256_xor_ps(a,_mm256_castsi256_ps(_mm256_set1_epi32(-1)))); }

		VMATH_INLINE BoolV& operator &=(ArgType b) { m_v = _mm256_and_ps(m_v,b); return *this; }
		VMATH_INLINE BoolV& operator |=(ArgType b) { m_v = _mm256_or_ps(m_v,b); return *this; }
		VMATH_INLINE BoolV& operator ^=(ArgType b) { m_v = _mm256_xor_ps(m_v,b); return *this; }

		IntrinsicType m_v;
	};
	VMATH_INLINE friend const BoolV operator ==(ArgType a,ArgType b) { return BoolV(_mm256_cmp_ps(a,b,_CMP_EQ_OQ)); }
	VMATH_INLINE friend const BoolV operator !=(ArgType a,ArgType b) { return BoolV(_mm256_cmp_ps(a,b,_CMP_NEQ_OQ)); }
	VMATH_INLINE friend const BoolV operator < (ArgType a,ArgType b) { return BoolV(_mm256_cmp_ps(a,b,_CMP_LT_OQ)); }
	VMATH_INLINE friend const BoolV operator <=(ArgType a,ArgType b) { return BoolV(_mm256_cmp_ps(a,b,_CMP_LE_OQ)); }
	VMATH_INLINE friend const BoolV operator > (ArgType a,ArgType b) { return BoolV(_mm256_cmp_ps(a,b,_CMP_GT_OQ)); }
	VMATH_INLINE friend const BoolV operator >=(ArgType a,ArgType b) { return BoolV(_mm256_cmp_ps(a,b,_CMP_GE_OQ)); }

	VMATH_INLINE friend const BoolV operator ==(ArgType a,ScalarV_arg b) { return BoolV(_mm256_cmp_ps(a,_vmath256_splatx_128_ps(b),_CMP_EQ_OQ)); }
	VMATH_INLINE friend const BoolV operator !=(ArgType a,ScalarV_arg b) { return BoolV(_mm256_cmp_ps(a,_vmath256_splatx_128_ps(b),_CMP_NEQ_OQ)); }
	VMATH_INLINE friend const BoolV operator < (ArgType a,ScalarV_arg b) { return BoolV(_mm256_cmp_ps(a,_vmath256_splatx_128_ps(b),_CMP_LT_OQ)); }
	VMATH_INLINE friend const BoolV operator <=(ArgType a,ScalarV_arg b) { return BoolV(_mm256_cmp_ps(a,_vmath256_splatx_128_ps(b),_CMP_LE_OQ)); }
	VMATH_INLINE friend const BoolV operator > (ArgType a,ScalarV_arg b) { return BoolV(_mm256_cmp_ps(a,_vmath256_splatx_128_ps(b),_CMP_GT_OQ)); }
	VMATH_INLINE friend const BoolV operator >=(ArgType a,ScalarV_arg b) { return BoolV(_mm256_cmp_ps(a,_vmath256_splatx_128_ps(b),_CMP_GE_OQ)); }

	VMATH_INLINE friend const BoolV operator ==(ScalarV_arg a,ArgType b) { return BoolV(_mm256_cmp_ps(_vmath256_splatx_128_ps(a),b,_CMP_EQ_OQ)); }
	VMATH_INLINE friend const BoolV operator !=(ScalarV_arg a,ArgType b) { return BoolV(_mm256_cmp_ps(_vmath256_splatx_128_ps(a),b,_CMP_NEQ_OQ)); }
	VMATH_INLINE friend const BoolV operator < (ScalarV_arg a,ArgType b) { return BoolV(_mm256_cmp_ps(_vmath256_splatx_128_ps(a),b,_CMP_LT_OQ)); }
	VMATH_INLINE friend const BoolV operator <=(ScalarV_arg a,ArgType b) { return BoolV(_mm256_cmp_ps(_vmath256_splatx_128_ps(a),b,_CMP_LE_OQ)); }
	VMATH_INLINE friend const BoolV operator > (ScalarV_arg a,ArgType b) { return BoolV(_mm256_cmp_ps(_vmath256_splatx_128_ps(a),b,_CMP_GT_OQ)); }
	VMATH_INLINE friend const BoolV operator >=(ScalarV_arg a,ArgType b) { return BoolV(_mm256_cmp_ps(_vmath256_splatx_128_ps(a),b,_CMP_GE_OQ)); }

	VMATH_INLINE friend const BoolV operator ==(ArgType a,ElementType b) { return BoolV(_mm256_cmp_ps(a,_mm256_set1_ps(b),_CMP_EQ_OQ)); }
	VMATH_INLINE friend const BoolV operator !=(ArgType a,ElementType b) { return BoolV(_mm256_cmp_ps(a,_mm256_set1_ps(b),_CMP_NEQ_OQ)); }
	VMATH_INLINE friend const BoolV operator < (ArgType a,ElementType b) { return BoolV(_mm256_cmp_ps(a,_mm256_set1_ps(b),_CMP_LT_OQ)); }
	VMATH_INLINE friend const BoolV operator <=(ArgType a,ElementType b) { return BoolV(_mm256_cmp_ps(a,_mm256_set1_ps(b),_CMP_LE_OQ)); }
	VMATH_INLINE friend const BoolV operator > (ArgType a,ElementType b) { return BoolV(_mm256_cmp_ps(a,_mm256_set1_ps(b),_CMP_GT_OQ)); }
	VMATH_INLINE friend const BoolV operator >=(ArgType a,ElementType b) { return BoolV(_mm256_cmp_ps(a,_mm256_set1_ps(b),_CMP_GE_OQ)); }

	VMATH_INLINE friend const BoolV operator ==(ElementType a,ArgType b) { return BoolV(_mm256_cmp_ps(_mm256_set1_ps(a),b,_CMP_EQ_OQ)); }
	VMATH_INLINE friend const BoolV operator !=(ElementType a,ArgType b) { return BoolV(_mm256_cmp_ps(_mm256_set1_ps(a),b,_CMP_NEQ_OQ)); }
	VMATH_INLINE friend const BoolV operator < (ElementType a,ArgType b) { return BoolV(_mm256_cmp_ps(_mm256_set1_ps(a),b,_CMP_LT_OQ)); }
	VMATH_INLINE friend const BoolV operator <=(ElementType a,ArgType b) { return BoolV(_mm256_cmp_ps(_mm256_set1_ps(a),b,_CMP_LE_OQ)); }
	VMATH_INLINE friend const BoolV operator > (ElementType a,ArgType b) { return BoolV(_mm256_cmp_ps(_mm256_set1_ps(a),b,_CMP_GT_OQ)); }
	VMATH_INLINE friend const BoolV operator >=(ElementType a,ArgType b) { return BoolV(_mm256_cmp_ps(_mm256_set1_ps(a),b,_CMP_GE_OQ)); }

	VMATH_INLINE friend OutType Select(ArgType a,ArgType b,typename BoolV::ArgType sel) { return T(_mm256_blendv_ps(a,b,sel)); }

private:
	IntrinsicType m_v;
};

typedef Vec8V::ArgType Vec8V_arg;
typedef Vec8V::OutType Vec8V_out;

#define VEC8V_ARGS(v) VEC4V_ARGS(v.v0()),VEC4V_ARGS(v.v1())

VMATH_INLINE void PrintV(const char* name,Vec8V_arg v) { printf("%s=%f,%f,%f,%f,%f,%f,%f,%f\n",name,VEC8V_ARGS(v)); }
VMATH_INLINE void PrintV(const char* name,typename Vec8V::IntrinsicType v) { PrintV(name,Vec8V(v)); }
#endif // HAS_VEC8V

#endif // _INCLUDE_VMATH_VEC8_H_

// =========================
// common/vmath/vmath_vec2.h
// =========================

#ifndef _INCLUDE_VMATH_VEC2_H_
#define _INCLUDE_VMATH_VEC2_H_

#include "vmath_common.h"
#include "vmath_floatvec.h"
#include "vmath_scalar.h"

class Vec2V
{
public:
	typedef Vec2V T;
	typedef const T& ArgType;
	typedef const T OutType;
	typedef __m128 IntrinsicType;
	typedef __m128i IntrinsicTypeInt;
	typedef float ElementType;
	typedef uint32 ElementTypeInt;
	enum { NumElements = 2 };

	VMATH_INLINE Vec2V() {}
	VMATH_INLINE explicit Vec2V(IntrinsicType v): m_v(v) {}
	VMATH_INLINE explicit Vec2V(IntrinsicTypeInt v): m_v(_mm_castsi128_ps(v)) {}
	VMATH_INLINE explicit Vec2V(ElementType f): m_v(_mm_set1_ps(f)) {}
#if PLATFORM_PC // TODO FIX PS4
	template <typename T> VMATH_INLINE Vec2V(const typename Vec2_T<T>& v): m_v(_mm_setr_ps(v.xf(),v.yf(),0.0f,0.0f)) {}
#endif // PLATFORM_PC
	VMATH_INLINE Vec2V(ScalarV::ArgType s): m_v(s) {}
	VMATH_INLINE Vec2V(ElementType x,ElementType y): m_v(_mm_setr_ps(x,y,0.0f,0.0f)) {}
	VMATH_INLINE Vec2V(int x,int y): m_v(_mm_cvtepi32_ps(_mm_setr_epi32(x,y,0,0))) {}
	VMATH_INLINE Vec2V(uint32 x,uint32 y): m_v(_mm_cvtepi32_ps(_mm_setr_epi32(x,y,0,0))) {}
	VMATH_INLINE Vec2V(ScalarV::ArgType x,ScalarV::ArgType y): m_v(_mm_unpacklo_ps(x,y)) {}
	VMATH_INLINE Vec2V(VectorConstantInitializer constant)
	{
		switch (constant) {
		case V_ZERO: m_v = _mm_setzero_ps(); break;
		case V_ONE: m_v = _mm_set1_ps(1.0f); break;
		case V_XAXIS: { const IntrinsicType one = _mm_set1_ps(1.0f); m_v = _mm_insert_ps(one,one,0x0F - 1); break; }
		case V_YAXIS: { const IntrinsicType one = _mm_set1_ps(1.0f); m_v = _mm_insert_ps(one,one,0x0F - 2); break; }
		default: DEBUG_ASSERT(false);
		}
	}

	VMATH_INLINE operator IntrinsicType() const { return m_v; }
	VMATH_INLINE IntrinsicType v() const { return m_v; }
	VMATH_INLINE IntrinsicType& v_ref() { return m_v; }

	VMATH_INLINE operator Vec2f() const { return *(const Vec2f*)this; }

	VMATH_INLINE ScalarV_out x() const { return ScalarV(_vmath_splatx_ps(m_v)); }
	VMATH_INLINE ScalarV_out y() const { return ScalarV(_vmath_splaty_ps(m_v)); }

	VMATH_INLINE ElementType xf() const { return _vmath_extract_ps(m_v,0); }
	VMATH_INLINE ElementType yf() const { return _vmath_extract_ps(m_v,1); }

	VMATH_INLINE ElementType& xf_ref() { return _vmath_ref_ps(m_v,0); }
	VMATH_INLINE ElementType& yf_ref() { return _vmath_ref_ps(m_v,1); }
	VMATH_INLINE const ElementType& xf_constref() const { return _vmath_constref_ps(m_v,0); }
	VMATH_INLINE const ElementType& yf_constref() const { return _vmath_constref_ps(m_v,1); }

	VMATH_INLINE void SetX(ScalarV_arg x) { m_v = _mm_blend_ps(m_v,x,VMATH_SELECT4(1,0,0,0)); }
	VMATH_INLINE void SetY(ScalarV_arg y) { m_v = _mm_blend_ps(m_v,y,VMATH_SELECT4(0,1,0,0)); }
	
	VMATH_INLINE ElementType operator [](unsigned index) const { DEBUG_ASSERT(index < NumElements); return _vmath_extract_ps(m_v,index); }
	VMATH_INLINE ElementType& operator [](unsigned index) { DEBUG_ASSERT(index < NumElements); return _vmath_ref_ps(m_v,index); }

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

	VMATH_INLINE friend ScalarV_out MinElement(ArgType a) { return ScalarV(_mm_min_ps(_vmath_splatx_ps(a),_vmath_splaty_ps(a))); }
	VMATH_INLINE friend ScalarV_out MaxElement(ArgType a) { return ScalarV(_mm_max_ps(_vmath_splatx_ps(a),_vmath_splaty_ps(a))); }

	VMATH_INLINE friend unsigned MinElementIndex(ArgType a)
	{
		const ScalarV m = MinElement(a); // TODO -- vectorize this better
		if (m == a.x()) return 0;
		else return 1;
	}
	VMATH_INLINE friend unsigned MaxElementIndex(ArgType a)
	{
		const ScalarV m = MaxElement(a); // TODO -- vectorize this better
		if (m == a.x()) return 0;
		else return 1;
	}

	VMATH_INLINE friend ScalarV_out Dot(ArgType a,ArgType b) { return ScalarV(_mm_dp_ps(a,b,0x3F)); }
	VMATH_INLINE friend ScalarV_out MagSqr(ArgType a) { return ScalarV(_mm_dp_ps(a,a,0x3F)); }
	VMATH_INLINE friend ScalarV_out Mag(ArgType a) { return ScalarV(_mm_sqrt_ps(_mm_dp_ps(a,a,0x3F))); }
	
	VMATH_INLINE friend ScalarV_out Cross(ArgType a,ArgType b) { return MultiplySub(a,b.y(),b*a.y()).x(); } // a.x*b.y - b.x*a.y

	VMATH_INLINE friend OutType Normalize(ArgType a) { return a*RecipSqrt(MagSqr(a)); }
	VMATH_INLINE friend OutType NormalizeSafe(ArgType a,ScalarV::ArgType epsilonSqr = ScalarV(V_ZERO))
	{
		const ScalarV aa = MagSqr(a);
		return Select(a*RecipSqrt(aa),T(V_ZERO),T(aa) <= T(epsilonSqr));
	}

	VMATH_INLINE static OutType SinCos(float angle) { return T(cosf(angle),sinf(angle)); }

	VMATH_INLINE static OutType FastSinCosPI_0_2(float normAngle) // very close approx to SinCos(normAngle*PI), for normAngle in the range [0..2]
	{
		DEBUG_ASSERT(0.0f <= normAngle && normAngle <= 2.0f);
		const T one(V_ONE);
		T x = T(Abs(normAngle - 1.0f) - 0.5f,1.0f - normAngle);
		T y = 4.0f*x*(one - Abs(x));
		y += y*ScalarV(0.224f)*(Abs(y) - one); // apply extra precision (optional)
		return y;
	}

	VMATH_INLINE static OutType LoadScalar(const float* src) { return T(_vmath_broadcast_ss(src)); }
	VMATH_INLINE static OutType Load(const float src[NumElements]) { return T(_mm_loadl_pi(_mm_setzero_ps(),reinterpret_cast<const __m64*>(src))); }
	VMATH_INLINE static OutType LoadFloat16V(const uint16 src[NumElements]) { return T(_mm_cvtph_ps(_mm_castps_si128(_mm_load_ss(reinterpret_cast<const float*>(src))))); }
	VMATH_INLINE static OutType LoadFixed16V(const uint16 src[NumElements])
	{
		const IntrinsicTypeInt temp = _mm_unpacklo_epi16(_mm_castps_si128(_mm_load_ss(reinterpret_cast<const float*>(src))),_mm_setzero_si128()); // {0,x, 0,y, 0,z, 0,w} in integer form
		const IntrinsicType scale = _mm_set1_ps(1.0f/65535.0f);
		return T(_mm_mul_ps(_mm_cvtepi32_ps(temp),scale));
	}
	VMATH_INLINE void Store(float dst[NumElements]) const { _mm_storel_pi(reinterpret_cast<__m64*>(dst),m_v); }
	VMATH_INLINE void StoreFloat16V(uint16 dst[NumElements]) const { _mm_store_ss(reinterpret_cast<float*>(dst),_mm_castsi128_ps(_mm_cvtps_ph(m_v,0))); }
	VMATH_INLINE void StoreFixed16V(uint16 dst[NumElements]) const
	{
		const IntrinsicType scale = _mm_set1_ps(65535.0f);
		const IntrinsicType bias = _mm_set1_ps(0.5f);
		const IntrinsicTypeInt temp = _mm_cvtps_epi32(_mm_fmadd_ps(m_v,scale,bias)); // temp = (int)(v*65535.0f + 0.5f)
		_mm_store_ss(reinterpret_cast<float*>(dst),_mm_castsi128_ps(_mm_packus_epi32(temp,temp)));
	}

	VMATH_INLINE friend bool IsFinite(ArgType a)
	{
		const IntrinsicTypeInt mask = _mm_set1_epi32(0x7F800000);
		return All(BoolV(_mm_cmpeq_epi32(_mm_and_si128(_mm_castps_si128(a),mask),mask)));
	}

	VMATH_INLINE friend bool AnyNaN(ArgType a)
	{
		return NotAll(BoolV(_mm_cmpeq_ps(a,a)));
	}

	class BoolV
	{
	public:
		static const uint32 MaskAll = (1 << NumElements) - 1;
		typedef const BoolV& ArgType;

		VMATH_INLINE explicit BoolV(IntrinsicType v): m_v(v) {}
		VMATH_INLINE explicit BoolV(IntrinsicTypeInt v): m_v(_mm_castsi128_ps(v)) {}
		VMATH_INLINE operator IntrinsicType() const { return m_v; }
		VMATH_INLINE uint32 GetMask() const { return _mm_movemask_ps(m_v) & MaskAll; }
		VMATH_INLINE friend bool All(ArgType a) { return (_mm_movemask_ps(a) & MaskAll) == MaskAll; }
		VMATH_INLINE friend bool Any(ArgType a) { return (_mm_movemask_ps(a) & MaskAll) != 0; }
		VMATH_INLINE friend bool None(ArgType a) { return (_mm_movemask_ps(a) & MaskAll) == 0; }
		VMATH_INLINE friend bool NotAll(ArgType a) { return (_mm_movemask_ps(a) & MaskAll) != MaskAll; }
		VMATH_INLINE static BoolV AllTrue() { return BoolV(_mm_set1_epi32(-1)); }
		VMATH_INLINE static BoolV AllFalse() { return BoolV(_mm_setzero_ps()); }

		VMATH_INLINE friend const BoolV operator ==(ArgType a,ArgType b) { return BoolV(_mm_castsi128_ps(_mm_cmpeq_epi32(_mm_castps_si128(a),_mm_castps_si128(b)))); }
		VMATH_INLINE friend const BoolV operator !=(ArgType a,ArgType b) { return BoolV(_mm_xor_ps(a,b)); }
		VMATH_INLINE friend const BoolV operator &&(ArgType a,ArgType b) { return BoolV(_mm_and_ps(a,b)); }
		VMATH_INLINE friend const BoolV operator ||(ArgType a,ArgType b) { return BoolV(_mm_or_ps(a,b)); }
		VMATH_INLINE friend const BoolV operator !(ArgType a) { return BoolV(_vmath_not_ps(a)); }

		VMATH_INLINE BoolV& operator &=(ArgType b) { m_v = _mm_and_ps(m_v,b); return *this; }
		VMATH_INLINE BoolV& operator |=(ArgType b) { m_v = _mm_or_ps(m_v,b); return *this; }
		VMATH_INLINE BoolV& operator ^=(ArgType b) { m_v = _mm_xor_ps(m_v,b); return *this; }

		IntrinsicType m_v;
	};
	VMATH_INLINE friend const BoolV operator ==(ArgType a,ArgType b) { return BoolV(_mm_cmpeq_ps(a,b)); }
	VMATH_INLINE friend const BoolV operator !=(ArgType a,ArgType b) { return BoolV(_mm_cmpneq_ps(a,b)); }
	VMATH_INLINE friend const BoolV operator < (ArgType a,ArgType b) { return BoolV(_mm_cmplt_ps(a,b)); }
	VMATH_INLINE friend const BoolV operator <=(ArgType a,ArgType b) { return BoolV(_mm_cmple_ps(a,b)); }
	VMATH_INLINE friend const BoolV operator > (ArgType a,ArgType b) { return BoolV(_mm_cmpgt_ps(a,b)); }
	VMATH_INLINE friend const BoolV operator >=(ArgType a,ArgType b) { return BoolV(_mm_cmpge_ps(a,b)); }

	VMATH_INLINE friend const BoolV operator ==(ArgType a,ScalarV_arg b) { return BoolV(_mm_cmpeq_ps(a,b)); }
	VMATH_INLINE friend const BoolV operator !=(ArgType a,ScalarV_arg b) { return BoolV(_mm_cmpneq_ps(a,b)); }
	VMATH_INLINE friend const BoolV operator < (ArgType a,ScalarV_arg b) { return BoolV(_mm_cmplt_ps(a,b)); }
	VMATH_INLINE friend const BoolV operator <=(ArgType a,ScalarV_arg b) { return BoolV(_mm_cmple_ps(a,b)); }
	VMATH_INLINE friend const BoolV operator > (ArgType a,ScalarV_arg b) { return BoolV(_mm_cmpgt_ps(a,b)); }
	VMATH_INLINE friend const BoolV operator >=(ArgType a,ScalarV_arg b) { return BoolV(_mm_cmpge_ps(a,b)); }

	VMATH_INLINE friend const BoolV operator ==(ScalarV_arg a,ArgType b) { return BoolV(_mm_cmpeq_ps(a,b)); }
	VMATH_INLINE friend const BoolV operator !=(ScalarV_arg a,ArgType b) { return BoolV(_mm_cmpneq_ps(a,b)); }
	VMATH_INLINE friend const BoolV operator < (ScalarV_arg a,ArgType b) { return BoolV(_mm_cmplt_ps(a,b)); }
	VMATH_INLINE friend const BoolV operator <=(ScalarV_arg a,ArgType b) { return BoolV(_mm_cmple_ps(a,b)); }
	VMATH_INLINE friend const BoolV operator > (ScalarV_arg a,ArgType b) { return BoolV(_mm_cmpgt_ps(a,b)); }
	VMATH_INLINE friend const BoolV operator >=(ScalarV_arg a,ArgType b) { return BoolV(_mm_cmpge_ps(a,b)); }

	VMATH_INLINE friend const BoolV operator ==(ArgType a,ElementType b) { return BoolV(_mm_cmpeq_ps(a,_mm_set1_ps(b))); }
	VMATH_INLINE friend const BoolV operator !=(ArgType a,ElementType b) { return BoolV(_mm_cmpneq_ps(a,_mm_set1_ps(b))); }
	VMATH_INLINE friend const BoolV operator < (ArgType a,ElementType b) { return BoolV(_mm_cmplt_ps(a,_mm_set1_ps(b))); }
	VMATH_INLINE friend const BoolV operator <=(ArgType a,ElementType b) { return BoolV(_mm_cmple_ps(a,_mm_set1_ps(b))); }
	VMATH_INLINE friend const BoolV operator > (ArgType a,ElementType b) { return BoolV(_mm_cmpgt_ps(a,_mm_set1_ps(b))); }
	VMATH_INLINE friend const BoolV operator >=(ArgType a,ElementType b) { return BoolV(_mm_cmpge_ps(a,_mm_set1_ps(b))); }

	VMATH_INLINE friend const BoolV operator ==(ElementType a,ArgType b) { return BoolV(_mm_cmpeq_ps(_mm_set1_ps(a),b)); }
	VMATH_INLINE friend const BoolV operator !=(ElementType a,ArgType b) { return BoolV(_mm_cmpneq_ps(_mm_set1_ps(a),b)); }
	VMATH_INLINE friend const BoolV operator < (ElementType a,ArgType b) { return BoolV(_mm_cmplt_ps(_mm_set1_ps(a),b)); }
	VMATH_INLINE friend const BoolV operator <=(ElementType a,ArgType b) { return BoolV(_mm_cmple_ps(_mm_set1_ps(a),b)); }
	VMATH_INLINE friend const BoolV operator > (ElementType a,ArgType b) { return BoolV(_mm_cmpgt_ps(_mm_set1_ps(a),b)); }
	VMATH_INLINE friend const BoolV operator >=(ElementType a,ArgType b) { return BoolV(_mm_cmpge_ps(_mm_set1_ps(a),b)); }

	VMATH_INLINE friend OutType Select(ArgType a,ArgType b,typename BoolV::ArgType sel) { return T(_mm_blendv_ps(a,b,sel)); }

	VMATH_INLINE friend OutType Select00(ArgType arg0,ArgType arg1) { return arg0; }
	VMATH_INLINE friend OutType Select01(ArgType arg0,ArgType arg1) { return T(_mm_blend_ps(arg0,arg1,VMATH_SELECT2(0,1))); }
	VMATH_INLINE friend OutType Select10(ArgType arg0,ArgType arg1) { return T(_mm_blend_ps(arg0,arg1,VMATH_SELECT2(1,0))); }
	VMATH_INLINE friend OutType Select11(ArgType arg0,ArgType arg1) { return arg1; }

private:
	IntrinsicType m_v;
	friend class Vec3V;
	friend class Vec4V;
};

typedef typename Vec2V::ArgType Vec2V_arg;
typedef typename Vec2V::OutType Vec2V_out;

#define VEC2V_ARGS(v) (v).xf(),(v).yf()

VMATH_INLINE void PrintV(const char* name,Vec2V_arg v) { printf("%s=%f,%f\n",name,VEC2V_ARGS(v)); }

#endif // _INCLUDE_VMATH_VEC2_H_

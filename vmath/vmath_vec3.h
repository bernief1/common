// =========================
// common/vmath/vmath_vec3.h
// =========================

#ifndef _INCLUDE_VMATH_VEC3_H_
#define _INCLUDE_VMATH_VEC3_H_

#include "vmath_common.h"
#include "vmath_floatvec.h"
#include "vmath_vec2.h"

class Vec3V
{
public:
	typedef Vec3V T;
	typedef const T& ArgType;
	typedef const T OutType;
	typedef __m128 IntrinsicType;
	typedef __m128i IntrinsicTypeInt;
	typedef float ElementType;
	typedef uint32 ElementTypeInt;
	enum { NumElements = 3 };

	// these are to make this compatible with some Vec3V_SOAn template code
	typedef ScalarV ComponentType;
	enum { NumVectors = ComponentType::NumElements };

	VMATH_INLINE Vec3V() {}
	VMATH_INLINE explicit Vec3V(IntrinsicType v): m_v(v) {}
	VMATH_INLINE explicit Vec3V(IntrinsicTypeInt v): m_v(_mm_castsi128_ps(v)) {}
	VMATH_INLINE explicit Vec3V(ElementType f): m_v(_mm_set1_ps(f)) {}
#if PLATFORM_PC // TODO FIX PS4
	template <typename T> VMATH_INLINE Vec3V(const typename Vec3_T<T>& v): m_v(_mm_setr_ps(v.xf(),v.yf(),v.zf(),0.0f)) {}
#endif // PLATFORM_PC
	VMATH_INLINE Vec3V(ScalarV::ArgType s): m_v(s) {}
	VMATH_INLINE Vec3V(ElementType x,ElementType y,ElementType z): m_v(_mm_setr_ps(x,y,z,0.0f)) {}
	VMATH_INLINE Vec3V(int x,int y,int z): m_v(_mm_cvtepi32_ps(_mm_setr_epi32(x,y,z,0))) {}
	VMATH_INLINE Vec3V(uint32 x,uint32 y,uint32 z): m_v(_mm_cvtepi32_ps(_mm_setr_epi32(x,y,z,0))) {}
	VMATH_INLINE Vec3V(Vec2V::ArgType xy,ElementType z): m_v(_mm_insert_ps(xy,_mm_set1_ps(z),2<<4)) {}
	VMATH_INLINE Vec3V(Vec2V::ArgType xy,ScalarV::ArgType z): m_v(_mm_insert_ps(xy,z,2<<4)) {} // could also use _vmath_permute_AABB_ps
	VMATH_INLINE Vec3V(ScalarV::ArgType x,ScalarV::ArgType y,ScalarV::ArgType z): m_v(_mm_setr_ps(x.f(),y.f(),z.f(),0.0f)) {} // optimize!
	VMATH_INLINE Vec3V(VectorConstantInitializer constant)
	{
		switch (constant) {
		case V_ZERO: m_v = _mm_setzero_ps(); break;
		case V_ONE: m_v = _mm_set1_ps(1.0f); break;
		case V_XAXIS: { const IntrinsicType one = _mm_set1_ps(1.0f); m_v = _mm_insert_ps(one,one,0x0F - 1); break; }
		case V_YAXIS: { const IntrinsicType one = _mm_set1_ps(1.0f); m_v = _mm_insert_ps(one,one,0x0F - 2); break; }
		case V_ZAXIS: { const IntrinsicType one = _mm_set1_ps(1.0f); m_v = _mm_insert_ps(one,one,0x0F - 4); break; }
		default: DEBUG_ASSERT(false);
		}
	}

	VMATH_INLINE operator IntrinsicType() const { return m_v; }
	VMATH_INLINE IntrinsicType v() const { return m_v; }
	VMATH_INLINE IntrinsicType& v_ref() { return m_v; }

	VMATH_INLINE operator Vec3f() const { return *(const Vec3f*)this; }

	VMATH_INLINE ScalarV_out x() const { return ScalarV(_vmath_splatx_ps(m_v)); }
	VMATH_INLINE ScalarV_out y() const { return ScalarV(_vmath_splaty_ps(m_v)); }
	VMATH_INLINE ScalarV_out z() const { return ScalarV(_vmath_splatz_ps(m_v)); }

	VMATH_INLINE ElementType xf() const { return _vmath_extract_ps(m_v,0); }
	VMATH_INLINE ElementType yf() const { return _vmath_extract_ps(m_v,1); }
	VMATH_INLINE ElementType zf() const { return _vmath_extract_ps(m_v,2); }

	VMATH_INLINE ElementType& xf_ref() { return _vmath_ref_ps(m_v,0); }
	VMATH_INLINE ElementType& yf_ref() { return _vmath_ref_ps(m_v,1); }
	VMATH_INLINE ElementType& zf_ref() { return _vmath_ref_ps(m_v,2); }
	VMATH_INLINE const ElementType& xf_constref() const { return _vmath_constref_ps(m_v,0); }
	VMATH_INLINE const ElementType& yf_constref() const { return _vmath_constref_ps(m_v,1); }
	VMATH_INLINE const ElementType& zf_constref() const { return _vmath_constref_ps(m_v,2); }

	VMATH_INLINE Vec2V_out xy() const { return Vec2V(m_v); }
	VMATH_INLINE Vec2V_out xz() const { return Vec2V(_vmath_permute_ps<0,2,0,2>(m_v)); }

	VMATH_INLINE Vec2f& xyf_ref() { return *reinterpret_cast<Vec2f*>(this); }

	VMATH_INLINE void SetX(ScalarV_arg x) { m_v = _mm_blend_ps(m_v,x,VMATH_SELECT4(1,0,0,0)); }
	VMATH_INLINE void SetY(ScalarV_arg y) { m_v = _mm_blend_ps(m_v,y,VMATH_SELECT4(0,1,0,0)); }
	VMATH_INLINE void SetZ(ScalarV_arg z) { m_v = _mm_blend_ps(m_v,z,VMATH_SELECT4(0,0,1,0)); }
	VMATH_INLINE void SetXY(Vec2V_arg xy) { m_v = _mm_blend_ps(m_v,xy,VMATH_SELECT4(1,1,0,0)); }

	VMATH_INLINE ElementType operator [](unsigned index) const { DEBUG_ASSERT_FMT(index < NumElements,"index=%u",index); return _vmath_extract_ps(m_v,index); }
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

	VMATH_INLINE friend ScalarV_out MinElement(ArgType a) { return ScalarV(_mm_min_ps(_mm_min_ps(_vmath_splatx_ps(a),_vmath_splaty_ps(a)),_vmath_splatz_ps(a))); }
	VMATH_INLINE friend ScalarV_out MaxElement(ArgType a) { return ScalarV(_mm_max_ps(_mm_max_ps(_vmath_splatx_ps(a),_vmath_splaty_ps(a)),_vmath_splatz_ps(a))); }

	VMATH_INLINE friend unsigned MinElementIndex(ArgType a)
	{
		const ScalarV m = MinElement(a); // TODO -- vectorize this better
		if (m == a.x()) return 0;
		else if (m == a.y()) return 1;
		else return 2;
	}
	VMATH_INLINE friend unsigned MaxElementIndex(ArgType a)
	{
		const ScalarV m = MaxElement(a); // TODO -- vectorize this better
		if (m == a.x()) return 0;
		else if (m == a.y()) return 1;
		else return 2;
	}

	VMATH_INLINE friend ScalarV_out Dot(ArgType a,ArgType b) { return ScalarV(_mm_dp_ps(a,b,0x7F)); }
	VMATH_INLINE friend ScalarV_out MagSqr(ArgType a) { return ScalarV(_mm_dp_ps(a,a,0x7F)); }
	VMATH_INLINE friend ScalarV_out Mag(ArgType a) { return ScalarV(_mm_sqrt_ps(_mm_dp_ps(a,a,0x7F))); }

	VMATH_INLINE friend OutType Cross(ArgType a,ArgType b)
	{
		const IntrinsicType a_yzxw = _vmath_permute_ps<1,2,0,3>(a); // a.yzxw
		const IntrinsicType b_yzxw = _vmath_permute_ps<1,2,0,3>(b); // b.yzxw
		const IntrinsicType temp = _mm_fnmadd_ps(b,a_yzxw,_mm_mul_ps(a,b_yzxw)); // a.xyzw*b.yzxw - b.xyzw*a.yzxw
		return T(_vmath_permute_ps<1,2,0,3>(temp)); // a.yzxw*b.zxyw - b.yzxw*a.zxyw
	}
	VMATH_INLINE friend OutType operator %(ArgType a,ArgType b) { return Cross(a,b); } // if you must

	VMATH_INLINE friend OutType Normalize(ArgType a) { return a*RecipSqrt(MagSqr(a)); }
	VMATH_INLINE friend OutType NormalizeSafe(ArgType a,ScalarV::ArgType epsilonSqr = ScalarV(V_ZERO))
	{
		const ScalarV aa = MagSqr(a);
		return Select(a*RecipSqrt(aa),T(V_ZERO),T(aa) <= T(epsilonSqr));
	}

	VMATH_INLINE static OutType LoadScalar(const float* src) { return T(_vmath_broadcast_ss(src)); }
	VMATH_INLINE static OutType Load(const float src[NumElements]) { return T(_mm_loadu_ps(src)); } // _mm_insert_ps(_mm_loadl_pi(m_v,reinterpret_cast<const __m64*>(src)),_mm_load_ss(src + 2),2<<4); }
	VMATH_INLINE static OutType LoadInt(const int src[NumElements]) { return T(_mm_cvtepi32_ps(_mm_loadu_si128(reinterpret_cast<const __m128i*>(src)))); }
	VMATH_INLINE static OutType LoadFloat16V(const uint16 src[NumElements]) { return T(_mm_cvtph_ps(_mm_cvtsi64_si128(*reinterpret_cast<const int64*>(src)))); } // note this accesses 2 bytes of memory beyond src[]
	VMATH_INLINE void Store(float dst[NumElements]) const { _mm_storel_pi(reinterpret_cast<__m64*>(dst),m_v); _mm_store_ss(dst + 2,_vmath_splatz_ps(m_v)); }
	VMATH_INLINE void StoreFloat16V(uint16 dst[NumElements]) const { const uint64 temp = _mm_cvtsi128_si64(_mm_cvtps_ph(m_v,0)); *reinterpret_cast<uint32*>(dst) = (uint32)temp; dst[2] = (uint16)(temp >> 32); }

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

	VMATH_INLINE friend OutType Select000(ArgType arg0,ArgType arg1) { return arg0; }
	VMATH_INLINE friend OutType Select001(ArgType arg0,ArgType arg1) { return T(_mm_blend_ps(arg0,arg1,VMATH_SELECT3(0,0,1))); }
	VMATH_INLINE friend OutType Select010(ArgType arg0,ArgType arg1) { return T(_mm_blend_ps(arg0,arg1,VMATH_SELECT3(0,1,0))); }
	VMATH_INLINE friend OutType Select011(ArgType arg0,ArgType arg1) { return T(_mm_blend_ps(arg0,arg1,VMATH_SELECT3(0,1,1))); }
	VMATH_INLINE friend OutType Select100(ArgType arg0,ArgType arg1) { return T(_mm_blend_ps(arg0,arg1,VMATH_SELECT3(1,0,0))); }
	VMATH_INLINE friend OutType Select101(ArgType arg0,ArgType arg1) { return T(_mm_blend_ps(arg0,arg1,VMATH_SELECT3(1,0,1))); }
	VMATH_INLINE friend OutType Select110(ArgType arg0,ArgType arg1) { return T(_mm_blend_ps(arg0,arg1,VMATH_SELECT3(1,1,0))); }
	VMATH_INLINE friend OutType Select111(ArgType arg0,ArgType arg1) { return arg1; }

private:
	IntrinsicType m_v;
	friend class Vec4V;
};

typedef typename Vec3V::ArgType Vec3V_arg;
typedef typename Vec3V::OutType Vec3V_out;

#define VEC3V_ARGS(v) (v).xf(),(v).yf(),(v).zf()

VMATH_INLINE void PrintV(const char* name,Vec3V_arg v) { printf("%s=%f,%f,%f\n",name,VEC3V_ARGS(v)); }

#endif // _INCLUDE_VMATH_VEC3_H_

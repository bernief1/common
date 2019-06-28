// =================================
// common/vmath/vmath_platform_sse.h
// =================================

#ifndef _INCLUDE_VMATH_PLATFORM_SSE_
#define _INCLUDE_VMATH_PLATFORM_SSE_

#ifndef VMATH_INCLUDING_FROM_VMATH_COMMON
#error "include vmath_common.h instead of this header!"
#endif

#include "vmath_pc.h"

#if !defined(__VMATH_FMA__)
#define _mm_fmadd_ss(a,b,c) _mm_add_ss(_mm_mul_ss(a,b),c) // a*b + c
#define _mm_fmsub_ss(a,b,c) _mm_sub_ss(_mm_mul_ss(a,b),c) // a*b - c
#define _mm_fnmadd_ss(a,b,c) _mm_sub_ss(c,_mm_mul_ss(a,b)) // -a*b + c
#define _mm_fnmsub_ss(a,b,c) _mm_sub_ss(_mm_setzero_ps(),_mm_add_ss(_mm_mul_ss(a,b),c)) // -(a*b + c)
#define _mm_fmadd_ps(a,b,c) _mm_add_ps(_mm_mul_ps(a,b),c) // a*b + c
#define _mm_fmsub_ps(a,b,c) _mm_sub_ps(_mm_mul_ps(a,b),c) // a*b - c
#define _mm_fnmadd_ps(a,b,c) _mm_sub_ps(c,_mm_mul_ps(a,b)) // -a*b + c
#define _mm_fnmsub_ps(a,b,c) _mm_sub_ps(_mm_setzero_ps(),_mm_add_ps(_mm_mul_ps(a,b),c)) // -(a*b + c)
#define _mm256_fmadd_ps(a,b,c) _mm256_add_ps(_mm256_mul_ps(a,b),c) // a*b + c
#define _mm256_fmsub_ps(a,b,c) _mm256_sub_ps(_mm256_mul_ps(a,b),c) // a*b - c
#define _mm256_fnmadd_ps(a,b,c) _mm256_sub_ps(c,_mm256_mul_ps(a,b)) // -a*b + c
#define _mm256_fnmsub_ps(a,b,c) _mm256_sub_ps(_mm256_setzero_ps(),_mm256_add_ps(_mm256_mul_ps(a,b),c)) // -(a*b + c)
#elif PLATFORM_PS4 // apparently we need to add support for feature 'fma4' to make these work .. but it wouldn't increase performance
#define _mm_fmadd_ss _mm_macc_ss
#define _mm_fmsub_ss _mm_msub_ss
#define _mm_fnmadd_ss _mm_nmacc_ss
#define _mm_fnmsub_ss _mm_nmsub_ss
#define _mm_fmadd_ps _mm_macc_ps
#define _mm_fmsub_ps _mm_msub_ps
#define _mm_fnmadd_ps _mm_nmacc_ps
#define _mm_fnmsub_ps _mm_nmsub_ps
#define _mm256_fmadd_ps _mm256_macc_ps
#define _mm256_fmsub_ps _mm256_msub_ps
#define _mm256_fnmadd_ps _mm256_nmacc_ps
#define _mm256_fnmsub_ps _mm256_nmsub_ps
#endif

#if PLATFORM_PC
#define _vmath_extract_ps(v,index) (v).m128_f32[index]
#define _vmath_ref_ps(v,index) (v).m128_f32[index]
#define _vmath_constref_ps(v,index) (v).m128_f32[index]
#define _vmath256_extract_ps(v,index) (v).m256_f32[index]
#define _vmath256_ref_ps(v,index) (v).m256_f32[index]
#define _vmath256_constref_ps(v,index) (v).m256_f32[index]
#else
#define _vmath_extract_ps(v,index) reinterpret_cast<const float*>(&(v))[index]
#define _vmath_ref_ps(v,index) reinterpret_cast<float*>(&(v))[index]
#define _vmath_constref_ps(v,index) reinterpret_cast<const float*>(&(v))[index]
#define _vmath256_extract_ps(v,index) reinterpret_cast<const float*>(&(v))[index]
#define _vmath256_ref_ps(v,index) reinterpret_cast<float*>(&(v))[index]
#define _vmath256_constref_ps(v,index) reinterpret_cast<const float*>(&(v))[index]
#endif

#define VMATH_SELECT2(x,y) ((x)|((y)<<1)) // for _mm_blend_ps etc.
#define VMATH_SELECT3(x,y,z) ((x)|((y)<<1)|((z)<<2))
#define VMATH_SELECT4(x,y,z,w) ((x)|((y)<<1)|((z)<<2)|((w)<<3))
#define VMATH_SELECT8(x0,y0,z0,w0,x1,y1,z1,w1) (VMATH_SELECT4(x0,y0,z0,w0)|(VMATH_SELECT4(x1,y1,z1,w1)<<4))
#define VMATH_PERMUTE(x,y,z,w) _MM_SHUFFLE(w,z,y,x) // operands reversed for clarity
#if defined(__AVX__)
template <unsigned x,unsigned y,unsigned z,unsigned w> VMATH_INLINE __m128 _vmath_permute_ps(__m128 a) { return _mm_permute_ps(a,VMATH_PERMUTE(x,y,z,w)); }
template <unsigned x,unsigned y,unsigned z,unsigned w> VMATH_INLINE __m128i _vmath_permute_epi32(__m128i a) { return _mm_castps_si128(_mm_permute_ps(_mm_castps_si128(a),VMATH_PERMUTE(x,y,z,w))); }
#else
template <unsigned x,unsigned y,unsigned z,unsigned w> VMATH_INLINE __m128 _vmath_permute_ps(__m128 a) { return _mm_shuffle_ps(a,a,VMATH_PERMUTE(x,y,z,w)); }
template <unsigned x,unsigned y,unsigned z,unsigned w> VMATH_INLINE __m128i _vmath_permute_epi32(__m128i a) { return _mm_shuffle_epi32(a,VMATH_PERMUTE(x,y,z,w)); }
#endif
template <unsigned x,unsigned y,unsigned z,unsigned w> VMATH_INLINE __m128 _vmath_permute_AABB_ps(__m128 a,__m128 b) { return _mm_shuffle_ps(a,b,VMATH_PERMUTE(x,y,z,w)); }
template <unsigned x,unsigned y,unsigned z,unsigned w> VMATH_INLINE __m128i _vmath_permute_AABB_epi32(__m128i a,__m128i b) { return _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(a),_mm_castsi128_ps(b),VMATH_PERMUTE(x,y,z,w))); }

#if defined(VISUAL_STUDIO_VERSION) && VISUAL_STUDIO_VERSION == 2017 // TODO -- why can't this work in 2015?
constexpr unsigned _vmath_shuffle_GetComponentFromPattern(uint32 pattern,unsigned i)
{
	const unsigned ch = (pattern>>(8*(3 - i)))&0xFF;
	if (ch == 'w')
		return 3;
	else
		return ch - 'x';
	// TODO -- compile-time error if pattern contains invalid characters
}
#define SHUFFLE_ARGS() \
	const unsigned x = _vmath_shuffle_GetComponentFromPattern(pattern,0); \
	const unsigned y = _vmath_shuffle_GetComponentFromPattern(pattern,1); \
	const unsigned z = _vmath_shuffle_GetComponentFromPattern(pattern,2); \
	const unsigned w = _vmath_shuffle_GetComponentFromPattern(pattern,3);
template <uint32 pattern> VMATH_INLINE __m128 _vmath_permute_XYZW_ps(__m128 a) { SHUFFLE_ARGS() return _mm_shuffle_ps(a,a,VMATH_PERMUTE(x,y,z,w)); }
template <uint32 pattern> VMATH_INLINE __m128i _vmath_permute_XYZW_epi32(__m128i a) { SHUFFLE_ARGS() return _mm_shuffle_epi32(a,a,VMATH_PERMUTE(x,y,z,w)); }
template <uint32 pattern> VMATH_INLINE __m128 _vmath_permute_AABB_XYZW_ps(__m128 a,__m128 b) { SHUFFLE_ARGS() return _mm_shuffle_ps(a,b,VMATH_PERMUTE(x,y,z,w)); }
template <uint32 pattern> VMATH_INLINE __m128i _vmath_permute_AABB_XYZW_epi32(__m128i a,__m128i b) { SHUFFLE_ARGS() return _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(a),_mm_castsi128_ps(b),VMATH_PERMUTE(x,y,z,w))); }
#undef SHUFFLE_ARGS
#endif

#if defined(__AVX2__)
#define _vmath_splatx_ps(a) _mm_broadcastss_ps(a)
#else
#define _vmath_splatx_ps(a) _vmath_permute_ps<0,0,0,0>(a)
#endif
#define _vmath_splaty_ps(a) _vmath_permute_ps<1,1,1,1>(a)
#define _vmath_splatz_ps(a) _vmath_permute_ps<2,2,2,2>(a)
#define _vmath_splatw_ps(a) _vmath_permute_ps<3,3,3,3>(a)

#if defined(__AVX__)
#if defined(__AVX2__)
#define _vmath256_splatx_128_ps _mm256_broadcastss_ps
#else
#define _vmath256_splatx_128_ps(a) _mm256_permute_ps(_mm256_castps128_ps256(a),VMATH_PERMUTE(0,0,0,0))
#endif
#define _vmath256_splaty_128_ps(a) _mm256_permute_ps(_mm256_castps128_ps256(a),VMATH_PERMUTE(1,1,1,1))
#define _vmath256_splatz_128_ps(a) _mm256_permute_ps(_mm256_castps128_ps256(a),VMATH_PERMUTE(2,2,2,2))
#define _vmath256_splatw_128_ps(a) _mm256_permute_ps(_mm256_castps128_ps256(a),VMATH_PERMUTE(3,3,3,3))
#define _vmath_broadcast_ss(a) _mm_broadcast_ss(a)
#define _vmath256_broadcast_ss(a) _mm256_broadcast_ss(a)
#else
#define _vmath_broadcast_ss(a) _vmath_splatx_ps(_mm_load_ss(a))
#define _vmath256_broadcast_ss(a) _vmath256_splatx_128_ps(_mm_load_ss(a))
#endif

#define _vmath_signbit_ps() _mm_castsi128_ps(_mm_set1_epi32(0x80000000))
#define _vmath_allones_ps() _mm_castsi128_ps(_mm_set1_epi32(0xFFFFFFFF))
#define _vmath_abs_ps(a) _mm_andnot_ps(_vmath_signbit_ps(),a)
#define _vmath_negate_ps(a) _mm_xor_ps(_vmath_signbit_ps(),a) // is this better than _mm_sub_ps(_mm_setzero_ps(),a)?
#define _vmath_not_ps(a) _mm_xor_ps(a,_vmath_allones_ps())
#define _vmath_floor_ps(a) _mm_round_ps(a,_MM_FROUND_TO_NEG_INF|_MM_FROUND_NO_EXC)//_mm_floor_ps(a)
#define _vmath_ceil_ps(a) _mm_round_ps(a,_MM_FROUND_TO_POS_INF|_MM_FROUND_NO_EXC)//_mm_ceil_ps(a)
#define _vmath_trunc_ps(a) _mm_round_ps(a,_MM_FROUND_TO_ZERO|_MM_FROUND_NO_EXC)
#define _vmath_round_ps(a) _mm_round_ps(a,_MM_FROUND_TO_NEAREST_INT|_MM_FROUND_NO_EXC)

#if PLATFORM_PC && defined(_WIN32) && !defined(_WIN64) // not sure why these are not available in 32-bit ..
VMATH_INLINE __m128i _mm_cvtsi64_si128(__int64 a) { __m128i v; v.m128i_i64[0] = a; return v; }
VMATH_INLINE __int64 _mm_cvtsi128_si64(__m128i a) { a.m128i_i64[0]; }
#endif // PLATFORM_PC && defined(_WIN32) && !defined(_WIN64)

#define VMATH_NEWTON_RAPHSON_RECIPROCAL_OPTIMIZATION (0)
// NOTE: raytracer was having issues with optimized reciprocal code because 0 -> NaN (instead of INF).
// we need to either have a separate codepath which allows for +/-0 inputs (and returns the appropriate
// +/-INF result) or manually handle +/-0 input ourselves.

VMATH_INLINE __m128 _vmath_recip_ps(__m128 a)
{
#if VMATH_NEWTON_RAPHSON_RECIPROCAL_OPTIMIZATION
	const __m128 two = _mm_set1_ps(2.0f);
#if defined(__AVX512VL__)
	__m128 est = _mm_rcp14_ps(a); // estimate
#else
	__m128 est = _mm_rcp_ps(a); // estimate
#endif
	est = _mm_mul_ps(est,_mm_fnmadd_ps(a,est,two)); // est <- est*(2 - a*est)
	// note that we could also do "_mm_fnmadd_ps(_mm_mul_ps(est,est),a,_mm_add_ps(est,est))" at the cost of 1 more instruction but no constant creation
	return est;
#else
	return _mm_div_ps(_mm_set1_ps(1.0f),a);
#endif
}

VMATH_INLINE __m128 _vmath_rsqrt_ps(__m128 a)
{
#if VMATH_NEWTON_RAPHSON_RECIPROCAL_OPTIMIZATION
	const __m128 half = _mm_set1_ps(0.5f);
	const __m128 three = _mm_set1_ps(3.0f);
#if defined(__AVX512VL__)
	__m128 est = _mm_rsqrt14_ps(a); // estimate
#else
	__m128 est = _mm_rsqrt_ps(a); // estimate
#endif
	est = _mm_mul_ps(_mm_mul_ps(est,half),_mm_fnmadd_ps(a,_mm_mul_ps(est,est),three)); // est <- est*(3 - a*est^2)/2
	return est;
#else
	return _mm_div_ps(_mm_set1_ps(1.0f),_mm_sqrt_ps(a));
#endif
}

#define _vmath_poly0_ps(x,c0)                   _mm_set1_ps(c0)
#define _vmath_poly1_ps(x,c0,c1)                _mm_fmadd_ps(_vmath_poly0_ps(x,c1               ),x,_mm_set1_ps(c0))
#define _vmath_poly2_ps(x,c0,c1,c2)             _mm_fmadd_ps(_vmath_poly1_ps(x,c1,c2            ),x,_mm_set1_ps(c0))
#define _vmath_poly3_ps(x,c0,c1,c2,c3)          _mm_fmadd_ps(_vmath_poly2_ps(x,c1,c2,c3         ),x,_mm_set1_ps(c0))
#define _vmath_poly4_ps(x,c0,c1,c2,c3,c4)       _mm_fmadd_ps(_vmath_poly3_ps(x,c1,c2,c3,c4      ),x,_mm_set1_ps(c0))
#define _vmath_poly5_ps(x,c0,c1,c2,c3,c4,c5)    _mm_fmadd_ps(_vmath_poly4_ps(x,c1,c2,c3,c4,c5   ),x,_mm_set1_ps(c0))
#define _vmath_poly6_ps(x,c0,c1,c2,c3,c4,c5,c6) _mm_fmadd_ps(_vmath_poly4_ps(x,c1,c2,c3,c4,c5,c6),x,_mm_set1_ps(c0))
#define VMATH_POLY5_PS(x,func) _vmath_poly5_ps(x,VMATH_##func##_POLY5_##C0,VMATH_##func##_POLY5_##C1,VMATH_##func##_POLY5_##C2,VMATH_##func##_POLY5_##C3,VMATH_##func##_POLY5_##C4,VMATH_##func##_POLY5_##C5)
#define VMATH_POLY4_PS(x,func) _vmath_poly4_ps(x,VMATH_##func##_POLY5_##C0,VMATH_##func##_POLY5_##C1,VMATH_##func##_POLY5_##C2,VMATH_##func##_POLY5_##C3,VMATH_##func##_POLY5_##C4)
#define VMATH_POLY3_PS(x,func) _vmath_poly3_ps(x,VMATH_##func##_POLY5_##C0,VMATH_##func##_POLY5_##C1,VMATH_##func##_POLY5_##C2,VMATH_##func##_POLY5_##C3)
#define VMATH_POLY2_PS(x,func) _vmath_poly2_ps(x,VMATH_##func##_POLY5_##C0,VMATH_##func##_POLY5_##C1,VMATH_##func##_POLY5_##C2)
#if HAS_VEC8V
#define _vmath256_poly0_ps(x,c0)                   _mm256_set1_ps(c0)
#define _vmath256_poly1_ps(x,c0,c1)                _mm256_fmadd_ps(_vmath256_poly0_ps(x,c1               ),x,_mm256_set1_ps(c0))
#define _vmath256_poly2_ps(x,c0,c1,c2)             _mm256_fmadd_ps(_vmath256_poly1_ps(x,c1,c2            ),x,_mm256_set1_ps(c0))
#define _vmath256_poly3_ps(x,c0,c1,c2,c3)          _mm256_fmadd_ps(_vmath256_poly2_ps(x,c1,c2,c3         ),x,_mm256_set1_ps(c0))
#define _vmath256_poly4_ps(x,c0,c1,c2,c3,c4)       _mm256_fmadd_ps(_vmath256_poly3_ps(x,c1,c2,c3,c4      ),x,_mm256_set1_ps(c0))
#define _vmath256_poly5_ps(x,c0,c1,c2,c3,c4,c5)    _mm256_fmadd_ps(_vmath256_poly4_ps(x,c1,c2,c3,c4,c5   ),x,_mm256_set1_ps(c0))
#define _vmath256_poly6_ps(x,c0,c1,c2,c3,c4,c5,c6) _mm256_fmadd_ps(_vmath256_poly4_ps(x,c1,c2,c3,c4,c5,c6),x,_mm256_set1_ps(c0))
#define VMATH256_POLY5_PS(x,func) _vmath256_poly5_ps(x,VMATH_##func##_POLY5_##C0,VMATH_##func##_POLY5_##C1,VMATH_##func##_POLY5_##C2,VMATH_##func##_POLY5_##C3,VMATH_##func##_POLY5_##C4,VMATH_##func##_POLY5_##C5)
#define VMATH256_POLY4_PS(x,func) _vmath256_poly4_ps(x,VMATH_##func##_POLY5_##C0,VMATH_##func##_POLY5_##C1,VMATH_##func##_POLY5_##C2,VMATH_##func##_POLY5_##C3,VMATH_##func##_POLY5_##C4)
#define VMATH256_POLY3_PS(x,func) _vmath256_poly3_ps(x,VMATH_##func##_POLY5_##C0,VMATH_##func##_POLY5_##C1,VMATH_##func##_POLY5_##C2,VMATH_##func##_POLY5_##C3)
#define VMATH256_POLY2_PS(x,func) _vmath256_poly2_ps(x,VMATH_##func##_POLY5_##C0,VMATH_##func##_POLY5_##C1,VMATH_##func##_POLY5_##C2)
#endif // HAS_VEC8V

VMATH_INLINE __m128i _vmath_mulhi_epu32(__m128i a, __m128i b)
{
	// no native _mulhi_epu32 instruction (or _epi32), so emulate it ..
	const __m128i c0 = _mm_srli_epi64(_mm_mul_epu32(a,b),32);
	const __m128i c1 = _mm_mul_epu32(_mm_srli_epi64(a,32),_mm_srli_epi64(b,32));
	return _mm_blend_epi32(c0,c1,VMATH_SELECT4(0,1,0,1));
}

#if HAS_VEC8V
VMATH_INLINE __m256i _vmath256_mulhi_epu32(__m256i a, __m256i b)
{
	// no native _mulhi_epu32 instruction (or _epi32), so emulate it ..
	const __m256i c0 = _mm256_srli_epi64(_mm256_mul_epu32(a,b),32);
	const __m256i c1 = _mm256_mul_epu32(_mm256_srli_epi64(a,32),_mm256_srli_epi64(b,32));
	return _mm256_blend_epi32(c0,c1,VMATH_SELECT8(0,1,0,1,0,1,0,1));
}
#endif // HAS_VEC8V

#endif // _INCLUDE_VMATH_PLATFORM_SSE_

// ===================================
// common/vmath/vmath_transcendental.h
// ===================================

#ifndef _INCLUDE_VMATH_TRANSCENDENTAL_H_
#define _INCLUDE_VMATH_TRANSCENDENTAL_H_

#include "vmath_common.h"
#include "vmath_vec4.h"

#if 0
	
	SinPI(x) := (x-1) * (x+1) * (((((0.000385937753182769*x^2 + -0.006860187425683514)*x^2  + 0.0751872634325299)*x^2  + -0.5240361513980939)*x^2  + 2.0261194642649887)*x^2  + -3.1415926444234477) * x;

	// calculate your own minimax?
	// https://common-lisp.net/~rtoy/maxima/minimax.mac
	// http://www.boost.org/doc/libs/1_36_0/libs/math/doc/sf_and_dist/html/math_toolkit/toolkit/internals2/minimax.html

	/* the following minimax polynomials come from some library .. */
	/* see __m128 exp2f4(__m128 x) */
	/* see __m128 log2f4(__m128 x) */

	/* minimax polynomials for 2^x in the range 0 <= x <= 1 (relative error) */
	exp2poly5(x):=x*(x*(x*(x*(x*.0018775767 + .0089893397) + .055826318) + .24015361) + .69315308) + .99999994;
	exp2poly4(x):=x*(x*(x*(x*.013534167 + .052011464) + .24144275) + .69300383) + 1.0000026;
	exp2poly3(x):=x*(x*(x*.078024521 + .22606716) + .69583356) + .99992520;
	exp2poly2(x):=x*(x*.33718944 + .65763628) + 1.0017247;
	
	/* show relative error */
	exp2(x):=2^x;
	wxplot2d([(exp2poly5(x) - exp2(x))/exp2(x)],[x,0,1]);
	wxplot2d([(exp2poly4(x) - exp2(x))/exp2(x)],[x,0,1]);
	wxplot2d([(exp2poly3(x) - exp2(x))/exp2(x)],[x,0,1]);
	wxplot2d([(exp2poly2(x) - exp2(x))/exp2(x)],[x,0,1]);

	/* minimax polynomials for log2(x)/(x - 1) in the range 1 <= x <= 2 (relative error) */
	log2overXminus1poly5(x):=x*(x*(x*(x*(x*-.0344359067839062357313 + .318212422185251071475) + -1.23152682416275988241) + 2.59883907202499966007) + -3.32419399085241980044) + 3.11578814719469302614;
	log2overXminus1poly4(x):=x*(x*(x*(x*.0596515482674574969533 + -.465725644288844778798) + 1.48116647521213171641) + -2.52074962577807006663) + 2.8882704548164776201;
	log2overXminus1poly3(x):=x*(x*(x*-.107254423828329604454 + .688243882994381274313) + -1.75647175389045657003) + 2.61761038894603480148;
	log2overXminus1poly2(x):=x*(x*.204446009836232697516 + -1.04913055217340124191) + 2.28330284476918490682;
	
	/* show relative error */
	log2(x):=log(x)/log(2);
	log2overXminus1(x):=log2(x)/(x - 1);
	log2poly5(x):=log2overXminus1poly5(x)*(x - 1);
	log2poly4(x):=log2overXminus1poly4(x)*(x - 1);
	log2poly3(x):=log2overXminus1poly3(x)*(x - 1);
	log2poly2(x):=log2overXminus1poly2(x)*(x - 1);
	wxplot2d([(log2poly5(x) - log2(x))/log2(x)],[x,1,2]);
	wxplot2d([(log2poly4(x) - log2(x))/log2(x)],[x,1,2]);
	wxplot2d([(log2poly3(x) - log2(x))/log2(x)],[x,1,2]);
	wxplot2d([(log2poly2(x) - log2(x))/log2(x)],[x,1,2]);

	// others ..
	// https://books.google.com/books?id=uLjSBwAAQBAJ&pg=PA283&lpg=PA283&dq=minimax+polynomial+exp2&source=bl&ots=KxZuxdVcZ3&sig=1DXaR9nmNBzV4N5Xk2WwwrYf_Gw&hl=en&sa=X&ved=0ahUKEwig-YiL9OTYAhWhY98KHeIqDkAQ6AEIZDAI#v=onepage&q=minimax%20polynomial%20exp2&f=false
	// this is good for E^x in the range [-2..2]
	// minimax max error is around 0.0032 (absolute)
	// might be good for cases where we don't need to handle high input ranges?
	// expEpoly5_alt2(x):=x*(x*(x*(x*(x*.01005370283 + .05072464725) + .1624711328) + .4860498435) + 1.002686427) + 1.003137731;
	
	// https://github.com/kristofe/GameEngine/blob/master/SupportLibraries/bullet-2.76/Extras/simdmathlibrary/ppu/simdmath/exp2f4.h
	// https://sourceforge.net/p/diplomkajn2014/code-0/7/tree//zdrojaky%20pro%20Honzu/FastFloat/pow.cpp?barediff=52d30cd57929e55dd59f0cdc:6
	// https://lists.freedesktop.org/archives/mesa-commit/2008-September/004489.html
#endif

// minimax for 2^x, input range 0 <= x <= 1, relative error
#define VMATH_EXP2_POLY5_C0 0.9999999400f
#define VMATH_EXP2_POLY5_C1 0.6931530800f
#define VMATH_EXP2_POLY5_C2 0.2401536100f
#define VMATH_EXP2_POLY5_C3 0.0558263180f
#define VMATH_EXP2_POLY5_C4 0.0089893397f
#define VMATH_EXP2_POLY5_C5 0.0018775767f

#define VMATH_EXP2_POLY4_C0 1.0000026000f
#define VMATH_EXP2_POLY4_C1 0.6930038300f
#define VMATH_EXP2_POLY4_C2 0.2414427500f
#define VMATH_EXP2_POLY4_C3 0.0520114640f
#define VMATH_EXP2_POLY4_C4 0.0135341670f

#define VMATH_EXP2_POLY3_C0 0.9999252000f
#define VMATH_EXP2_POLY3_C1 0.6958335600f
#define VMATH_EXP2_POLY3_C2 0.2260671600f
#define VMATH_EXP2_POLY3_C3 0.0780245210f

#define VMATH_EXP2_POLY2_C0 1.0017247000f
#define VMATH_EXP2_POLY2_C1 0.6576362800f
#define VMATH_EXP2_POLY2_C2 0.3371894400f

template <unsigned order> VMATH_INLINE __m128 _vmath_exp2_ps_0_1_(__m128 x); // minimax for 2^x, input range 0 <= x <= 1, relative error

template <> VMATH_INLINE __m128 _vmath_exp2_ps_0_1_<5>(__m128 x) { return VMATH_POLY5_PS(x,EXP2); }
template <> VMATH_INLINE __m128 _vmath_exp2_ps_0_1_<4>(__m128 x) { return VMATH_POLY4_PS(x,EXP2); }
template <> VMATH_INLINE __m128 _vmath_exp2_ps_0_1_<3>(__m128 x) { return VMATH_POLY3_PS(x,EXP2); }
template <> VMATH_INLINE __m128 _vmath_exp2_ps_0_1_<2>(__m128 x) { return VMATH_POLY2_PS(x,EXP2); }

template <unsigned order> VMATH_INLINE __m128 _vmath_exp2_ps_(__m128 x)
{
	// input range is assumed to be [-126..+129]
	const __m128i i = _mm_cvtps_epi32(_mm_sub_ps(x,_mm_set1_ps(0.5f))); // i = (int)floor(x) = int(x - 0.5)
	const __m128 f = _mm_sub_ps(x,_mm_cvtepi32_ps(i)); // f = x - i
	const __m128 ei = _mm_castsi128_ps(_mm_slli_epi32(_mm_add_epi32(i,_mm_set1_epi32(127)),23)); // ei = (float)(1 << i)
	const __m128 ef = _vmath_exp2_ps_0_1_<order>(f);
	return _mm_mul_ps(ei,ef);
}

VMATH_INLINE __m128 _vmath_exp2_ps(__m128 x) { return _vmath_exp2_ps_<5>(x); } // default order = 5 (~13x faster than using 4x exp2f)

#if HAS_VEC8V
template <unsigned order> VMATH_INLINE __m256 _vmath256_exp2_ps_0_1_(__m256 x);

template <> VMATH_INLINE __m256 _vmath256_exp2_ps_0_1_<5>(__m256 x) { return VMATH256_POLY5_PS(x,EXP2); }
template <> VMATH_INLINE __m256 _vmath256_exp2_ps_0_1_<4>(__m256 x) { return VMATH256_POLY4_PS(x,EXP2); }
template <> VMATH_INLINE __m256 _vmath256_exp2_ps_0_1_<3>(__m256 x) { return VMATH256_POLY3_PS(x,EXP2); }
template <> VMATH_INLINE __m256 _vmath256_exp2_ps_0_1_<2>(__m256 x) { return VMATH256_POLY2_PS(x,EXP2); }

template <unsigned order> VMATH_INLINE __m256 _vmath256_exp2_ps_(__m256 x)
{
	// input range is assumed to be [-126..+129]
	const __m256i i = _mm256_cvtps_epi32(_mm256_sub_ps(x,_mm256_set1_ps(0.5f))); // i = (int)floor(x) = int(x - 0.5)
	const __m256 f = _mm256_sub_ps(x,_mm256_cvtepi32_ps(i)); // f = x - i
	const __m256 ei = _mm256_castsi256_ps(_mm256_slli_epi32(_mm256_add_epi32(i,_mm256_set1_epi32(127)),23)); // ei = (float)(1 << i)
	const __m256 ef = _vmath256_exp2_ps_0_1_<order>(f);
	return _mm256_mul_ps(ei,ef);
}

VMATH_INLINE __m256 _vmath256_exp2_ps(__m256 x) { return _vmath256_exp2_ps_<5>(x); } // default order = 5 (~22x faster than using 8x exp2f)
#endif // HAS_VEC8V

// minimax for log2(x)/(x - 1), input range 1 <= x <= 2, relative error
// this effectively increases the polynomial degree by one, but ensures that log2(1) == 0
// note that these minimax polynomials are relative error, but that makes no sense with the log function. the way
// it is broken down into integer and fractional parts is such that you will always be absolute error bound,
// not relative error bound. so i think it would make more sense to use absolute error minimax polynomials here
#define VMATH_LOG2_POLY5_C0 +3.1157881471946930261400f
#define VMATH_LOG2_POLY5_C1 -3.3241939908524198004400f
#define VMATH_LOG2_POLY5_C2 +2.5988390720249996600700f
#define VMATH_LOG2_POLY5_C3 -1.2315268241627598824100f
#define VMATH_LOG2_POLY5_C4 +0.3182124221852510714750f
#define VMATH_LOG2_POLY5_C5 -0.0344359067839062357313f

#define VMATH_LOG2_POLY4_C0 +2.8882704548164776201000f
#define VMATH_LOG2_POLY4_C1 -2.5207496257780700666300f
#define VMATH_LOG2_POLY4_C2 +1.4811664752121317164100f
#define VMATH_LOG2_POLY4_C3 -0.4657256442888447787980f
#define VMATH_LOG2_POLY4_C4 +0.0596515482674574969533f

#define VMATH_LOG2_POLY3_C0 +2.6176103889460348014800f
#define VMATH_LOG2_POLY3_C1 -1.7564717538904565700300f
#define VMATH_LOG2_POLY3_C2 +0.6882438829943812743130f
#define VMATH_LOG2_POLY3_C3 -0.1072544238283296044540f

#define VMATH_LOG2_POLY2_C0 +2.2833028447691849068200f
#define VMATH_LOG2_POLY2_C1 -1.0491305521734012419100f
#define VMATH_LOG2_POLY2_C2 +0.2044460098362326975160f

template <unsigned order> VMATH_INLINE __m128 _vmath_log2_div_x_minus_1_ps_1_2_(__m128 x);

template <> VMATH_INLINE __m128 _vmath_log2_div_x_minus_1_ps_1_2_<5>(__m128 x) { return VMATH_POLY5_PS(x,LOG2); }
template <> VMATH_INLINE __m128 _vmath_log2_div_x_minus_1_ps_1_2_<4>(__m128 x) { return VMATH_POLY4_PS(x,LOG2); }
template <> VMATH_INLINE __m128 _vmath_log2_div_x_minus_1_ps_1_2_<3>(__m128 x) { return VMATH_POLY3_PS(x,LOG2); }
template <> VMATH_INLINE __m128 _vmath_log2_div_x_minus_1_ps_1_2_<2>(__m128 x) { return VMATH_POLY2_PS(x,LOG2); }

template <unsigned order> VMATH_INLINE __m128 _vmath_log2_ps_(__m128 x)
{
	const __m128i exp = _mm_set1_epi32(0x7F800000);
	const __m128i mant = _mm_set1_epi32(0x007FFFFF);
	const __m128 one = _mm_set1_ps(1.0f);
	const __m128i i = _mm_castps_si128(x);
	const __m128 e = _mm_cvtepi32_ps(_mm_sub_epi32(_mm_srli_epi32(_mm_and_si128(i,exp),23),_mm_set1_epi32(127)));
	const __m128 m = _mm_or_ps(_mm_castsi128_ps(_mm_and_si128(i, mant)),one);
	const __m128 p = _vmath_log2_div_x_minus_1_ps_1_2_<order>(m);
	return _mm_add_ps(_mm_mul_ps(p,_mm_sub_ps(m,one)),e);
}

VMATH_INLINE __m128 _vmath_log2_ps(__m128 x) { return _vmath_log2_ps_<5>(x); } // default order = 5 (~8x faster than using 4x log2f)

#if HAS_VEC8V
template <unsigned order> VMATH_INLINE __m256 _vmath256_log2_div_x_minus_1_ps_1_2_(__m256 x);

template <> VMATH_INLINE __m256 _vmath256_log2_div_x_minus_1_ps_1_2_<5>(__m256 x) { return VMATH256_POLY5_PS(x,LOG2); }
template <> VMATH_INLINE __m256 _vmath256_log2_div_x_minus_1_ps_1_2_<4>(__m256 x) { return VMATH256_POLY4_PS(x,LOG2); }
template <> VMATH_INLINE __m256 _vmath256_log2_div_x_minus_1_ps_1_2_<3>(__m256 x) { return VMATH256_POLY3_PS(x,LOG2); }
template <> VMATH_INLINE __m256 _vmath256_log2_div_x_minus_1_ps_1_2_<2>(__m256 x) { return VMATH256_POLY2_PS(x,LOG2); }

template <unsigned order> VMATH_INLINE __m256 _vmath256_log2_ps_(__m256 x)
{
	const __m256i exp = _mm256_set1_epi32(0x7F800000);
	const __m256i mant = _mm256_set1_epi32(0x007FFFFF);
	const __m256 one = _mm256_set1_ps(1.0f);
	const __m256i i = _mm256_castps_si256(x);
	const __m256 e = _mm256_cvtepi32_ps(_mm256_sub_epi32(_mm256_srli_epi32(_mm256_and_si256(i,exp),23),_mm256_set1_epi32(127)));
	const __m256 m = _mm256_or_ps(_mm256_castsi256_ps(_mm256_and_si256(i, mant)),one);
	const __m256 p = _vmath256_log2_div_x_minus_1_ps_1_2_<order>(m);
	return _mm256_add_ps(_mm256_mul_ps(p,_mm256_sub_ps(m,one)),e);
}

VMATH_INLINE __m256 _vmath256_log2_ps(__m256 x) { return _vmath256_log2_ps_<5>(x); } // default order = 5 (~15x faster than using 8x log2f)
#endif // HAS_VEC8V

VMATH_INLINE float FastExp(float x)
{
	// https://codingforspeed.com/using-faster-exponential-approximation/
	// approximate solution to E^x for values of x less than around 5 ..
	float y = 1.0f + x/1024.0f;
	y *= y; y *= y; y *= y; y *= y; y *= y;
	y *= y; y *= y; y *= y; y *= y; y *= y;
	return y;
}

VMATH_INLINE float FastExp2(float x)
{
	// approximate solution to 2^x for values of x less than around 5 ..
	const float ln2 = 0.69314718056f; // logf(2.0f)
	return FastExp(x*ln2);
}

VMATH_INLINE float FastSin(float x)
{
	// http://forum.devmaster.net/t/fast-and-accurate-sine-cosine/9648
	// accuracy is about +/-0.000924
	DEBUG_ASSERT(-PI <= x && x <= PI);
	const float B = 4.0f/PI;
	const float C = 4.0f/(PI*PI);
	float y = x*(B - C*Abs(x));
	const float P = 0.224f; // <-- 0.224 gives slightly better precision than 0.225
	y += y*P*(Abs(y) - 1.0f); // apply extra precision
	return y;
}

VMATH_INLINE float FastSinPI(float x) // approximation of sinf(x*PI) for -1 <= x <= 1
{
	DEBUG_ASSERT(-1.0f <= x && x <= 1.0f);
	float y = 4.0f*x*(1.0f - Abs(x));
	y += y*0.224f*(Abs(y) - 1.0f); // apply extra precision (optional)
	return y;
}

VMATH_INLINE float FastCosPI(float x) // approximation of cosf(x*PI) for -1.5 <= x <= 1.5
{
	return FastSinPI(0.5f - Abs(x));
}

VMATH_INLINE float FastSinPI_0_2(float x) // approximation of sinf(x*PI) for 0 <= x <= 2
{
	return FastSinPI(1.0f - x);
}

VMATH_INLINE float FastCosPI_0_2(float x) // approximation of cosf(x*PI) for -0.5 <= x <= 2.5
{
	return FastSinPI(Abs(1.0f - x) - 0.5f);
}

template <typename T> VMATH_INLINE typename T::OutType FastSinPI_T(const T& x)
{
	DEBUG_ASSERT(All(-1.0f <= x && x <= 1.0f));
	T y = 4.0f*x*(T(V_ONE) - Abs(x));
	y += y*0.224f*(Abs(y) - T(V_ONE)); // apply extra precision (optional)
	return y;
}

VMATH_INLINE ScalarV_out FastSinPI(ScalarV_arg x) { return ScalarV(FastSinPI(x.f())); }
VMATH_INLINE Vec2V_out FastSinPI(Vec2V_arg x) { return FastSinPI_T<Vec2V>(x); }
VMATH_INLINE Vec3V_out FastSinPI(Vec3V_arg x) { return FastSinPI_T<Vec3V>(x); }
VMATH_INLINE Vec4V_out FastSinPI(Vec4V_arg x) { return FastSinPI_T<Vec4V>(x); }

VMATH_INLINE ScalarV_out FastCosPI(ScalarV_arg x) { return ScalarV(FastCosPI(x.f())); }
VMATH_INLINE Vec2V_out FastCosPI(Vec2V_arg x) { return FastSinPI_T<Vec2V>(Vec2V(0.5f) - Abs(x)); }
VMATH_INLINE Vec3V_out FastCosPI(Vec3V_arg x) { return FastSinPI_T<Vec3V>(Vec3V(0.5f) - Abs(x)); }
VMATH_INLINE Vec4V_out FastCosPI(Vec4V_arg x) { return FastSinPI_T<Vec4V>(Vec4V(0.5f) - Abs(x)); }

VMATH_INLINE ScalarV FastSinPI_0_2(ScalarV_arg x) { return ScalarV(FastSinPI_0_2(x.f())); }
VMATH_INLINE Vec2V_out FastSinPI_0_2(Vec2V_arg x) { return FastSinPI_T<Vec2V>(Vec2V(V_ONE) - x); }
VMATH_INLINE Vec3V_out FastSinPI_0_2(Vec3V_arg x) { return FastSinPI_T<Vec3V>(Vec3V(V_ONE) - x); }
VMATH_INLINE Vec4V_out FastSinPI_0_2(Vec4V_arg x) { return FastSinPI_T<Vec4V>(Vec4V(V_ONE) - x); }

VMATH_INLINE ScalarV_out FastCosPI_0_2(ScalarV_arg x) { return ScalarV(FastCosPI_0_2(x.f())); }
VMATH_INLINE Vec2V_out FastCosPI_0_2(Vec2V_arg x) { return FastSinPI_T<Vec2V>(Abs(Vec2V(V_ONE) - x) - Vec2V(0.5f)); }
VMATH_INLINE Vec3V_out FastCosPI_0_2(Vec3V_arg x) { return FastSinPI_T<Vec3V>(Abs(Vec3V(V_ONE) - x) - Vec3V(0.5f)); }
VMATH_INLINE Vec4V_out FastCosPI_0_2(Vec4V_arg x) { return FastSinPI_T<Vec4V>(Abs(Vec4V(V_ONE) - x) - Vec4V(0.5f)); }

#define VMATH_SINPI_POLY5_C0 -3.141592644423447700f
#define VMATH_SINPI_POLY5_C1 +2.026119464264988700f
#define VMATH_SINPI_POLY5_C2 -0.524036151398093900f
#define VMATH_SINPI_POLY5_C3 +0.075187263432529900f
#define VMATH_SINPI_POLY5_C4 -0.006860187425683514f
#define VMATH_SINPI_POLY5_C5 +0.000385937753182769f

VMATH_INLINE __m128 _vmath_sinPI_ps(__m128 x) // sin(x*PI) for -1 <= x <= 1 (~3.5x faster than sinf(x*PI))
{
	// http://mooooo.ooo/chebyshev-sine-approximation/
	const __m128 one = _mm_set1_ps(1.0f);
	const __m128 temp1 = _mm_mul_ps(x,_mm_mul_ps(_mm_sub_ps(x,one),_mm_add_ps(x,one))); // x*(x-1)*(x+1)
	const __m128 temp2 = VMATH_POLY5_PS(_mm_mul_ps(x,x),SINPI);
	return _mm_mul_ps(temp1,temp2);
}

VMATH_INLINE float _vmath_sinPI(float x) // sin(x*PI) for -1 <= x <= 1 (only about ~1.35x faster than 4x sinf(x*PI))
{
	// http://mooooo.ooo/chebyshev-sine-approximation/
	DEBUG_ASSERT(-1.0f <= x && x <= 1.0f);
	const float x2 = x*x;
	float result;
	result = VMATH_SINPI_POLY5_C5;
	result = VMATH_SINPI_POLY5_C4 + result*x2;
	result = VMATH_SINPI_POLY5_C3 + result*x2;
	result = VMATH_SINPI_POLY5_C2 + result*x2;
	result = VMATH_SINPI_POLY5_C1 + result*x2;
	result = VMATH_SINPI_POLY5_C0 + result*x2;
	return result*x*(x - 1.0f)*(x + 1.0f);
}

#if HAS_VEC8V
VMATH_INLINE __m256 _vmath256_sinPI_ps(__m256 x) // sin(x*PI) for -1 <= x <= 1 (~6.15x faster than 8x sinf(x*PI))
{
	// http://mooooo.ooo/chebyshev-sine-approximation/
	const __m256 one = _mm256_set1_ps(1.0f);
	const __m256 temp1 = _mm256_mul_ps(x,_mm256_mul_ps(_mm256_sub_ps(x,one),_mm256_add_ps(x,one))); // x*(x-1)*(x+1)
	const __m256 temp2 = VMATH256_POLY5_PS(_mm256_mul_ps(x,x),SINPI);
	return _mm256_mul_ps(temp1,temp2);
}
#endif // HAS_VEC8V

#endif // _INCLUDE_VMATH_TRANSCENDENTAL_H_

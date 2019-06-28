// =============================
// common/vmath/vmath_sampling.h
// =============================

#ifndef _INCLUDE_VMATH_SAMPLING_H_
#define _INCLUDE_VMATH_SAMPLING_H_

#include "vmath_common.h"
#include "vmath_vec3.h"

VMATH_INLINE uint8 BitReverse8(uint8 i)
{
	i = ((i & 0x55) << 1) | ((i & ~0x55) >> 1);
	i = ((i & 0x33) << 2) | ((i & ~0x33) >> 2);
	return (i << 4) | (i >> 4);
}

VMATH_INLINE uint16 BitReverse16(uint16 i)
{
	i = ((i & 0x5555) << 1) | ((i & ~0x5555) >> 1);
	i = ((i & 0x3333) << 2) | ((i & ~0x3333) >> 2);
	i = ((i & 0x0F0F) << 4) | ((i & ~0x0F0F) >> 4);
	return (i << 8) | (i >> 8);
}

VMATH_INLINE uint32 BitReverse32(uint32 i)
{
	i = ((i & 0x55555555UL) << 1) | ((i & ~0x55555555UL) >> 1);
	i = ((i & 0x33333333UL) << 2) | ((i & ~0x33333333UL) >> 2);
	i = ((i & 0x0F0F0F0FUL) << 4) | ((i & ~0x0F0F0F0FUL) >> 4);
	i = ((i & 0x00FF00FFUL) << 8) | ((i & ~0x00FF00FFUL) >> 8);
	return (i << 16) | (i >> 16);
}

VMATH_INLINE uint64 BitReverse64(uint64 i)
{
	i = ((i & 0x5555555555555555ULL) << 0x01) | ((i & ~0x5555555555555555ULL) >> 0x01);
	i = ((i & 0x3333333333333333ULL) << 0x02) | ((i & ~0x3333333333333333ULL) >> 0x02);
	i = ((i & 0x0F0F0F0F0F0F0F0FULL) << 0x04) | ((i & ~0x0F0F0F0F0F0F0F0FULL) >> 0x04);
	i = ((i & 0x00FF00FF00FF00FFULL) << 0x08) | ((i & ~0x00FF00FF00FF00FFULL) >> 0x08);
	i = ((i & 0x0000FFFF0000FFFFULL) << 0x10) | ((i & ~0x0000FFFF0000FFFFULL) >> 0x10);
	return (i << 32) | (i >> 32);
}

#if PLATFORM_PC // TODO FIX PS4
VMATH_INLINE __m128i BitReverse128(__m128i i)
{
	__m128i j;
	j.m128i_u64[0] = BitReverse64(i.m128i_u64[1]);
	j.m128i_u64[1] = BitReverse64(i.m128i_u64[0]);
	return j;
}

VMATH_INLINE __m128i BitReverse32x4(__m128i i)
{
	const __m128i m1 = _mm_set1_epi8(0x55);
	const __m128i m2 = _mm_set1_epi8(0x33);
	const __m128i m3 = _mm_set1_epi8(0x0F);
	const __m128i m4 = _mm_set1_epi16(0x00FF);
	i = _mm_or_si128(_mm_slli_epi32(_mm_and_si128(m1,i),1),_mm_srli_epi32(_mm_andnot_si128(m1,i),1));
	i = _mm_or_si128(_mm_slli_epi32(_mm_and_si128(m2,i),2),_mm_srli_epi32(_mm_andnot_si128(m2,i),2));
	i = _mm_or_si128(_mm_slli_epi32(_mm_and_si128(m3,i),4),_mm_srli_epi32(_mm_andnot_si128(m3,i),4));
	i = _mm_or_si128(_mm_slli_epi32(_mm_and_si128(m4,i),8),_mm_srli_epi32(_mm_andnot_si128(m4,i),8));
	i = _mm_or_si128(_mm_slli_epi32(i,16),_mm_srli_epi32(i,16));
	return i;
}

#if HAS_VEC8V
VMATH_INLINE __m256i BitReverse32x8(__m256i i)
{
	const __m256i m1 = _mm256_set1_epi8(0x55);
	const __m256i m2 = _mm256_set1_epi8(0x33);
	const __m256i m3 = _mm256_set1_epi8(0x0F);
	const __m256i m4 = _mm256_set1_epi16(0x00FF);
	i = _mm256_or_si256(_mm256_slli_epi32(_mm256_and_si256(m1,i),1),_mm256_srli_epi32(_mm256_andnot_si256(m1,i),1));
	i = _mm256_or_si256(_mm256_slli_epi32(_mm256_and_si256(m2,i),2),_mm256_srli_epi32(_mm256_andnot_si256(m2,i),2));
	i = _mm256_or_si256(_mm256_slli_epi32(_mm256_and_si256(m3,i),4),_mm256_srli_epi32(_mm256_andnot_si256(m3,i),4));
	i = _mm256_or_si256(_mm256_slli_epi32(_mm256_and_si256(m4,i),8),_mm256_srli_epi32(_mm256_andnot_si256(m4,i),8));
	i = _mm256_or_si256(_mm256_slli_epi32(i,16),_mm256_srli_epi32(i,16));
	return i;
}
#endif // HAS_VEC8V
#endif // PLATFORM_PC

float HaltonSequence(uint32 i,uint32 b);
float RadicalInverseBase2(float x);
void GenerateHaltonSequence(float* dst,uint32 start,uint32 count,uint32 b,bool vectorized = true);
void GenerateHaltonSequence2D(Vec2f* dst,uint32 start,uint32 count,uint32 bx = 2,uint32 by = 3,bool vectorized = true);

Vec2V_out Hammersley2D(uint32 i,uint32 count);
Vec3V_out HammersleyHemisphere(uint32 i,uint32 count,bool uniform = true,float hemisphereAngle = 180.0f); // oriented towards Z
Vec3V_out HammersleyHemisphereOriented(uint32 i,uint32 count,Vec3V_arg N,bool uniform = true,float hemisphereAngle = 180.0f);
Vec3V_out HammersleyHemisphereOriented2(uint32 i,uint32 count,Vec3V_arg N,float uniformExp = 1.0f,float hemisphereAngle = 180.0f);

#endif // _INCLUDE_VMATH_SAMPLING_H_

// ===============================
// common/vmath/vmath_sampling.cpp
// ===============================

#include "vmath_matrix.h"
#include "vmath_sampling.h"
#include "vmath_transcendental.h"
#if PLATFORM_PC // TODO FIX PS4
#include "vmath_intdiv.h"
#endif // PLATFORM_PC

float HaltonSequence(uint32 i,uint32 b)
{
	// https://en.wikipedia.org/wiki/Halton_sequence
	const float q = 1.0f/(float)b;
	float f = 1.0f;
	float r = 0.0f;
	while (i > 0) {
		f *= q;
		r += f*(float)(i%b);
		i /= b;
	}
	return r;
}

float RadicalInverseBase2(float x)
{
	if (x > 0.0f && x < 1.0f) {
		x += 1.0f; // (1..2)
		uint32 xi = *(const uint32*)&x;
		xi = BitReverse32(xi & 0x007FFFFF) >> 9; // bit-reversed mantissa
		xi |= 0x3F800000UL; // bitwise-OR bits from 1.0f
		x = *(const float*)&xi;
		x -= 1.0f; // (0..1)
	}
	return x;
}

#define HALTON_SEQ_VECTOR_VALIDATE (1)

#if HALTON_SEQ_VECTOR_VALIDATE
#define HALTON_SEQ_VECTOR_VALIDATE_ONLY(...) __VA_ARGS__
#else
#define HALTON_SEQ_VECTOR_VALIDATE_ONLY(...)
#endif

void GenerateHaltonSequence(float* dst,uint32 start,uint32 count,uint32 b,bool vectorized)
{
#if PLATFORM_PC // TODO FIX PS4
	if (vectorized) {
		__m128i i4 = _mm_setr_epi32(start,start + 1,start + 2,start + 3);
		const __m128i di4 = _mm_set1_epi32(4);
		if (b == 2) {
			const __m128 scale32 = _mm_set1_ps(1.0f/4294967296.0f);
			for (uint32 i_ = 0; i_ + 4 <= count; i_ += 4) {
				const __m128 r = _mm_mul_ps(_mm_cvtepi32_ps(BitReverse32x4(i4)),scale32);
				_mm_storeu_ps(dst,r);
				dst += 4;
				i4 = _mm_add_epi32(i4,di4);
			}
		} else {
			const __m128i bv = _mm_set1_epi32(b);
			const __m128i bm = _mm_set1_epi32(IntegerDiv_GetMultiplierForFixedShift(b,32 HALTON_SEQ_VECTOR_VALIDATE_ONLY(,start + (count&~3))));
			const __m128 qv = _mm_set1_ps(1.0f/(float)b);
			const __m128i zero = _mm_setzero_si128();
			const __m128 one_f = _mm_set1_ps(1.0f);
			for (uint32 i_ = 0; i_ + 4 <= count; i_ += 4) {
				__m128i i = i4;
				__m128 f = one_f;
				__m128 r = _mm_castsi128_ps(zero);
				while (_mm_movemask_epi8(_mm_cmpeq_epi32(i,zero)) != 0xFFFF) {
					f = _mm_mul_ps(f,qv);
					const __m128i i_div_b = _vmath_mulhi_epu32(i,bm);
					const __m128i i_mod_b = _mm_sub_epi32(i,_mm_mullo_epi32(i_div_b,bv));
				#if HALTON_SEQ_VECTOR_VALIDATE
					for (uint32 k = 0; k < 4; k++) {
						ForceAssertf(i_div_b.m128i_u32[k] == i.m128i_u32[k]/b,"%u div %u -> %u, should be %u!",i.m128i_u32[k],b,i_div_b.m128i_u32[k],i.m128i_u32[k]/b);
						ForceAssertf(i_mod_b.m128i_u32[k] == i.m128i_u32[k]%b,"%u div %u -> %u, should be %u!",i.m128i_u32[k],b,i_mod_b.m128i_u32[k],i.m128i_u32[k]%b);
					}
				#endif // HALTON_SEQ_VECTOR_VALIDATE
					i = i_div_b;
					r = _mm_add_ps(r,_mm_mul_ps(f,_mm_cvtepi32_ps(i_mod_b)));
				}
				_mm_storeu_ps(dst,r);
				dst += 4;
				i4 = _mm_add_epi32(i4,di4);
			}
		}
		if (count&3)
			GenerateHaltonSequence(dst,start + (count&~3),count&3,b,false); // use non-vectorized to fill in the remanining
	} else
#endif // PLATFORM_PC
	{
		const uint32 end = start + count;
		if (b == 2) {
			for (uint32 i = start; i < end; i++) {
				dst[i - start] = (float)BitReverse32(i)/4294967296.0f;
			}
		} else {
			for (uint32 i = start; i < end; i++)
				dst[i - start] = HaltonSequence(i,b);
		}
	}
}

void GenerateHaltonSequence2D(Vec2f* dst,uint32 start,uint32 count,uint32 bx,uint32 by,bool vectorized)
{
	if (count < 8)
		vectorized = false; // TODO -- my vectorized code is crashing for small counts .. fix it
	float* hx = new float[count];
	float* hy = new float[count];
	GenerateHaltonSequence(hx,start,count,bx,vectorized);
	GenerateHaltonSequence(hy,start,count,by,vectorized);
	for (uint32 i = 0; i < count; i++)
		dst[i] = Vec2f(hx[i], hy[i]);
	delete[] hx;
	delete[] hy;
}

Vec2V_out Hammersley2D(uint32 i,uint32 count)
{
	const float x = (float)BitReverse32(i)*(1.0f/(float)(1ULL<<32)); // base-2 radical inverse
	const float y = (float)i/(float)(count - 1); // [0..1]
	return Vec2V(x,y);
}

VMATH_INLINE static float SqrKeepSign(float x)
{
	if (x >= 0.0f) return +x*x;
	else           return -x*x;
}

VMATH_INLINE static float SqrtKeepSign(float x)
{
	if (x >= 0.0f) return +Sqrt(+x);
	else           return -Sqrt(-x);
}

VMATH_INLINE static float PowKeepSign(float x,float y)
{
	if      (y == 0.0f) return x;
	else if (x >= 0.0f) return +powf(+x,y);
	else                return -powf(-x,y);
}

// this version doesn't work well with non-uniform and hemisphere angles != 180
Vec3V_out HammersleyHemisphere(uint32 i,uint32 count,bool uniform,float hemisphereAngle) // oriented towards Z
{
	DEBUG_ASSERT(hemisphereAngle >= 0.0f && hemisphereAngle <= 360.0f); // 360 = full sphere, 180 = hemisphere, 0 = single ray
	float wrap = 1.0f;
	if (hemisphereAngle != 180.0f) {
		wrap = Cos(hemisphereAngle*PI/360.0f);
		if (!uniform)
			wrap = SqrKeepSign(wrap);
		wrap = 1.0f - wrap;
	}
	const Vec2V xy = Hammersley2D(i,count + 1); // note: passing count + 1 here so that phi ranges [0..1), not [0..1]
	const float x = xy.xf();
	const float y = xy.yf();
	const Vec2V sinCosPhi = Vec2V::FastSinCosPI_0_2(2.0f*y);
	const float sinPhi = sinCosPhi.yf();
	const float cosPhi = sinCosPhi.xf();
	float cosTheta = 1.0f - wrap*x;
	if (!uniform)
		cosTheta = SqrtKeepSign(cosTheta);
	const float sinTheta = Sqrt(1.0f - cosTheta*cosTheta);
	const Vec3V V = Vec3V(cosPhi*sinTheta,sinPhi*sinTheta,cosTheta);
	return V;
}

Vec3V_out HammersleyHemisphereOriented(uint32 i,uint32 count,Vec3V_arg N,bool uniform,float hemisphereAngle)
{
	return Mat33V::ConstructBasis(N).Transform(HammersleyHemisphere(i,count,uniform,hemisphereAngle));
}

// TODO -- this one doesn't work with hemisphereAngle > 180, but it could be fixed
// also, the behavior of non-uniform (uniformPow != 1) with hemisphere angles != 180 is better here
Vec3V_out HammersleyHemisphereOriented2(uint32 i,uint32 count,Vec3V_arg N,float uniformExp,float hemisphereAngle) // oriented towards normal, cone angle scaled by A
{
	if (Any(N != Vec3V(V_ZAXIS)) && uniformExp != 1.0f) // won't work with N != {0,0,1}, do the transform separately ..
		return Mat33V::ConstructBasis(N).Transform(HammersleyHemisphereOriented2(i,count,Vec3V(V_ZAXIS),uniformExp,hemisphereAngle));
	DEBUG_ASSERT(hemisphereAngle >= 0.0f && hemisphereAngle <= 360.0f);
	const Vec2V xy = Hammersley2D(i,count + 1); // note: passing count + 1 here so that phi ranges [0..1), not [0..1]
	const float x = xy.xf();
	const float y = xy.yf();
	const Vec2V sinCosPhi = Vec2V::FastSinCosPI_0_2(2.0f*y);
	const float sinPhi = sinCosPhi.yf();
	const float cosPhi = sinCosPhi.xf();
	float cosTheta = 1.0f - x; // 1 - x for uniform, sqrt(abs(1 - x))*sign(1 - x) for cosine-distribution
	if (uniformExp != 1.0f)
		cosTheta = PowKeepSign(cosTheta,uniformExp);
	const float sinTheta = Sqrt(1.0f - cosTheta*cosTheta);
	Vec3V V = Vec3V(cosPhi*sinTheta,sinPhi*sinTheta,cosTheta);
	float NV = Dot(N,V).f();
	if (NV < 0.0f) { // necessary if N != {0,0,1}
		V = -V;
		NV = -NV;
	}
	if (hemisphereAngle != 180.0f) {
		const float c2 = Sqr(Cos(acosf(NV)*hemisphereAngle/180.0f));
		const float s2 = 1.0f - c2;
		const float z = Sqrt((c2/s2)*(1.0f - NV*NV)) - NV;
		V = Normalize(V + N*z);
	}
	return V;
}
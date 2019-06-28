// =============================
// common/vmath/vmath_random.cpp
// =============================

#include "vmath_random.h"

Vec4V_out XorShift4V_LRL_32_13_17_5_Init(uint32 seed,uint32 k0,uint32 k1,uint32 k2,uint32 k3)
{
	DEBUG_ASSERT(seed != 0);
	const uint32 x0 = XorShift_LRL_32_13_17_5_SkipAhead(seed,k0);
	const uint32 x1 = XorShift_LRL_32_13_17_5_SkipAhead(seed,k1);
	const uint32 x2 = XorShift_LRL_32_13_17_5_SkipAhead(seed,k2);
	const uint32 x3 = XorShift_LRL_32_13_17_5_SkipAhead(seed,k3);
	return Vec4V(_mm_setr_epi32(x0,x1,x2,x3));
}

Vec4V_out XorShift4V_RLR_31_11_13_20_Init(uint32 seed,uint32 k0,uint32 k1,uint32 k2,uint32 k3)
{
	DEBUG_ASSERT((seed & ~0x80000000) != 0);
	const uint32 x0 = XorShift_RLR_31_11_13_20_SkipAhead(seed,k0);
	const uint32 x1 = XorShift_RLR_31_11_13_20_SkipAhead(seed,k1);
	const uint32 x2 = XorShift_RLR_31_11_13_20_SkipAhead(seed,k2);
	const uint32 x3 = XorShift_RLR_31_11_13_20_SkipAhead(seed,k3);
	return Vec4V(_mm_setr_epi32(x0,x1,x2,x3));
}

#if HAS_VEC8V
Vec8V_out XorShift8V_LRL_32_13_17_5_Init(uint32 seed,uint32 k0,uint32 k1,uint32 k2,uint32 k3,uint32 k4,uint32 k5,uint32 k6,uint32 k7)
{
	DEBUG_ASSERT(seed != 0);
	const uint32 x0 = XorShift_LRL_32_13_17_5_SkipAhead(seed,k0);
	const uint32 x1 = XorShift_LRL_32_13_17_5_SkipAhead(seed,k1);
	const uint32 x2 = XorShift_LRL_32_13_17_5_SkipAhead(seed,k2);
	const uint32 x3 = XorShift_LRL_32_13_17_5_SkipAhead(seed,k3);
	const uint32 x4 = XorShift_LRL_32_13_17_5_SkipAhead(seed,k4);
	const uint32 x5 = XorShift_LRL_32_13_17_5_SkipAhead(seed,k5);
	const uint32 x6 = XorShift_LRL_32_13_17_5_SkipAhead(seed,k6);
	const uint32 x7 = XorShift_LRL_32_13_17_5_SkipAhead(seed,k7);
	return Vec8V(_mm256_setr_epi32(x0,x1,x2,x3,x4,x5,x6,x7));
}

Vec8V_out XorShift8V_RLR_31_11_13_20_Init(uint32 seed,uint32 k0,uint32 k1,uint32 k2,uint32 k3,uint32 k4,uint32 k5,uint32 k6,uint32 k7)
{
	DEBUG_ASSERT((seed & ~0x80000000) != 0);
	const uint32 x0 = XorShift_RLR_31_11_13_20_SkipAhead(seed,k0);
	const uint32 x1 = XorShift_RLR_31_11_13_20_SkipAhead(seed,k1);
	const uint32 x2 = XorShift_RLR_31_11_13_20_SkipAhead(seed,k2);
	const uint32 x3 = XorShift_RLR_31_11_13_20_SkipAhead(seed,k3);
	const uint32 x4 = XorShift_RLR_31_11_13_20_SkipAhead(seed,k4);
	const uint32 x5 = XorShift_RLR_31_11_13_20_SkipAhead(seed,k5);
	const uint32 x6 = XorShift_RLR_31_11_13_20_SkipAhead(seed,k6);
	const uint32 x7 = XorShift_RLR_31_11_13_20_SkipAhead(seed,k7);
	return Vec8V(_mm256_setr_epi32(x0,x1,x2,x3,x4,x5,x6,x7));
}
#endif // HAS_VEC8V

template <typename T,int N> class GF2Matrix // NxN Galois Field binary matrix
{
	static_assert(N <= sizeof(T)*8,"");
public:
	VMATH_INLINE GF2Matrix(T (*gen)(T))
	{
		for (int i = 0; i < N; i++)
			m_v[i] = gen(T(1) << i);
	}

	VMATH_INLINE const T ExpMul(T x,uint32 k) const // faster than calling Exp() and then Mul()
	{
		GF2Matrix<T,N> b = *this;
		while (k) {
			if (k & 1)
				x = b.Mul(x);
			k >>= 1;
			b.Sqr();
		}
		return x;
	}

private:
	T m_v[N];

	VMATH_INLINE void SetIdentity()
	{
		for (int i = 0; i < N; i++)
			m_v[i] = T(1) << i;
	}

	VMATH_INLINE static const T BitSplat(T x,uint32 i)
	{
		return (T)(((typename IntegerType_T<T>::T_signed(x))<<(N - 1 - i))>>(N - 1));
	}

	VMATH_INLINE const T Mul(T b) const
	{
		T y = T(0);
		for (int i = 0; i < N; i++)
			y ^= (m_v[i] & BitSplat(b,i));
		return y;
	}

	VMATH_INLINE void Mul(const GF2Matrix<T,N>& b)
	{
		DEBUG_ASSERT(this != &b);
		for (int i = 0; i < N; i++)
			m_v[i] = b.Mul(m_v[i]);
	}

	VMATH_INLINE void Sqr()
	{
		GF2Matrix<T,N> b = *this;
		Mul(b);
	}

	VMATH_INLINE void Exp(uint32 k)
	{
		GF2Matrix<T,N> b = *this;
		SetIdentity();
		while (k) {
			if (k & 1)
				Mul(b);
			k >>= 1;
			b.Sqr();
		}
	}
};

uint32 XorShift_LRL_32_13_17_5_SkipAhead(uint32 x,uint32 k)
{
	return GF2Matrix<uint32,32>(XorShift_LRL<uint32,32,13,17,5>).ExpMul(x,k); 
}

uint32 XorShift_RLR_31_11_13_20_SkipAhead(uint32 x,uint32 k)
{
	return GF2Matrix<uint32,32>(XorShift_RLR<uint32,31,11,13,20>).ExpMul(x,k); 
}

uint32 FindXorShiftsUsingExactlyNBits(uint32 N)
{
	// https://www.jstatsoft.org/article/view/v008i14/xorshift.pdf
	// note that if (a,b,c) produces a maximal period XorShift, then all of these also produce maximal period:
	// XorShiftLRL(a,b,c)
	// XorShiftLRL(c,b,a)
	// XorShiftRLR(a,b,c)
	// XorShiftRLR(c,b,a)
	// XorShiftLLR(a,c,b)
	// XorShiftLLR(c,a,b)
	// XorShiftRRL(a,c,b)
	// XorShiftRRL(c,a,b)
	// 
	// example:
	// 24-bit XorShift(1,5,18) has maximal period 16777215
	// 24-bit XorShift(1,7,9) has maximal period 16777215
	// 24-bit XorShift(1,9,13) has maximal period 16777215
	// 24-bit XorShift(1,11,20) has maximal period 16777215
	// 24-bit XorShift(1,17,12) has maximal period 16777215
	// 24-bit XorShift(2,3,19) has maximal period 16777215
	// 24-bit XorShift(2,11,13) has maximal period 16777215
	// 24-bit XorShift(3,7,5) has maximal period 16777215
	// 24-bit XorShift(3,7,9) has maximal period 16777215
	// 24-bit XorShift(3,9,19) has maximal period 16777215
	// 24-bit XorShift(3,11,16) has maximal period 16777215
	// 24-bit XorShift(3,15,11) has maximal period 16777215
	// 24-bit XorShift(3,21,7) has maximal period 16777215
	// 24-bit XorShift(3,21,16) has maximal period 16777215
	// 24-bit XorShift(4,3,15) has maximal period 16777215
	// 24-bit XorShift(4,11,15) has maximal period 16777215
	// 24-bit XorShift(5,9,11) has maximal period 16777215
	// 24-bit XorShift(5,9,12) has maximal period 16777215
	// 24-bit XorShift(5,11,15) has maximal period 16777215
	// 24-bit XorShift(5,13,17) has maximal period 16777215
	// 24-bit XorShift(6,1,9) has maximal period 16777215
	// 24-bit XorShift(6,5,11) has maximal period 16777215
	// 24-bit XorShift(6,11,15) has maximal period 16777215
	// 24-bit XorShift(8,1,11) has maximal period 16777215
	// 24-bit XorShift(8,15,11) has maximal period 16777215
	// 24-bit XorShift(8,15,13) has maximal period 16777215
	// 24-bit XorShift(9,5,10) has maximal period 16777215
	// 24-bit XorShift(11,1,19) has maximal period 16777215
	// 24-bit XorShift(11,13,14) has maximal period 16777215
	// 24-bit XorShift(13,11,22) has maximal period 16777215
	// found 30 24-bit XorShifts with maximal period 16777215
	const uint32 mask = BIT(N) - 1; // also maximal period
	uint32 count = 0;
	for (uint32 a = 1; a < N; a++) {
		for (uint32 b = 1; b < N; b++) {
			for (uint32 c = a + 1; c < N; c++) { // WLOG c > a
				// try XorShiftRLR(a,b,c) ..
				uint32 x = 1;
				for (uint32 i = 0; i < mask; i++) {
					x ^= (x >> a);
					x ^= (x << b); x &= mask;
					x ^= (x >> c);
					if (x == 1) {
						if (i == mask - 1) {
							printf("%u-bit XorShift(%u,%u,%u) has maximal period %u\n", N, a, b, c, mask);
							count++;
						}
						break;
					}
				}
			}
		}
	}
	printf("found %u %u-bit XorShifts with maximal period %u\n", count, N, mask);
	return count;
}

static uint32 g_RandomState = 1;

float GetRandomValue(uint32* state)
{
	if (state == nullptr)
		state = &g_RandomState;
	return XorShift_LRL_32_13_17_5(*state);
}

float GetRandomValueInRange(float minValue,float maxValue,uint32* state)
{
	return minValue + (maxValue - minValue)*GetRandomValue(state);
}

Vec2V_out GetRandomPoint2D(Vec2V_arg bmin,Vec2V_arg bmax,uint32* state)
{
	const float x = GetRandomValueInRange(bmin.xf(),bmax.xf(),state);
	const float y = GetRandomValueInRange(bmin.yf(),bmax.yf(),state);
	return Vec2V(x,y);
}

Vec3V_out GetRandomPoint3D(Vec3V_arg bmin,Vec3V_arg bmax,uint32* state)
{
	const float x = GetRandomValueInRange(bmin.xf(),bmax.xf(),state);
	const float y = GetRandomValueInRange(bmin.yf(),bmax.yf(),state);
	const float z = GetRandomValueInRange(bmin.zf(),bmax.zf(),state);
	return Vec3V(x,y,z);
}

Vec3V_out GetRandomDirection3D(uint32* state)
{
	Vec3V result;
	do {
		const float x = GetRandomValueInRange(-1.0f,1.0f,state);
		const float y = GetRandomValueInRange(-1.0f,1.0f,state);
		const float z = GetRandomValueInRange(-1.0f,1.0f,state);
		result = Vec3V(x, y, z);
	} while (MagSqr(result) < 0.1f);
	return Normalize(result);
}

Mat33V_out GetRandomRotation3D(uint32* state)
{
	const Vec3V dir = GetRandomDirection3D(state);
	Vec3V upDir;
	do {
		upDir = GetRandomDirection3D(state);
	} while (MagSqr(Cross(dir, upDir)) < 0.1f);
	return Mat33V::ConstructBasisUp(dir,upDir);
}
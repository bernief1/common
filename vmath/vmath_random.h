// ===========================
// common/vmath/vmath_random.h
// ===========================

#ifndef _INCLUDE_VMATH_RANDOM_H_
#define _INCLUDE_VMATH_RANDOM_H_

#include "vmath_common.h"
#include "vmath_matrix.h"
#include "vmath_vec8.h"

VMATH_INLINE Vec4V_out GetFloatValueFromRandomBits(__m128i bits)
{
	__m128 result = _mm_castsi128_ps(_mm_srli_epi32(bits,9)); // shift bits into mantissa
	const __m128 one = _mm_set1_ps(1.0f);
	result = _mm_or_ps(result,one); // bitwise-OR 1.0f so range is now [1..2) in floating-point
	result = _mm_sub_ps(result,one); // subtract 1.0f, range is [0..1)
	return Vec4V(result);
}

#if HAS_VEC8V
VMATH_INLINE Vec8V_out GetFloatValueFromRandomBits(__m256i bits)
{
	__m256 result = _mm256_castsi256_ps(_mm256_srli_epi32(bits,9)); // shift bits into mantissa
	const __m256 one = _mm256_set1_ps(1.0f);
	result = _mm256_or_ps(result,one); // bitwise-OR 1.0f so range is now [1..2) in floating-point
	result = _mm256_sub_ps(result,one); // subtract 1.0f, range is [0..1)
	return Vec8V(result);
}
#endif // HAS_VEC8V

VMATH_INLINE float GetFloatValueFromRandomBits(uint32 bits)
{
	uint32 result = bits >> 9; // shift bits into mantissa
	const float one = 1.0f;
	result |= *reinterpret_cast<const uint32*>(&one); // bitwise-OR 1.0f so range is now [1..2) in floating-point
	return *reinterpret_cast<const float*>(&result) - one; // subtract 1.0f, range is [0..1)
}

// https://www.jstatsoft.org/article/view/v008i14/xorshift.pdf "Xorshift RNGs - Journal of Statistical Software"
// https://en.wikipedia.org/wiki/XorShift
// https://arxiv.org/pdf/1204.6193.pdf "Random number generators for massively parallel simulations on GPU"

Vec4V_out XorShift4V_LRL_32_13_17_5_Init(uint32 seed=1,uint32 k0=0,uint32 k1=100000000,uint32 k2=200000000,uint32 k3=300000000);
Vec4V_out XorShift4V_RLR_31_11_13_20_Init(uint32 seed=1,uint32 k0=0,uint32 k1=100000000,uint32 k2=200000000,uint32 k3=300000000);
#if HAS_VEC8V
Vec8V_out XorShift8V_LRL_32_13_17_5_Init(uint32 seed=1,uint32 k0=0,uint32 k1=100000000,uint32 k2=200000000,uint32 k3=300000000,uint32 k4=400000000,uint32 k5=500000000,uint32 k6=600000000,uint32 k7=700000000);
Vec8V_out XorShift8V_RLR_31_11_13_20_Init(uint32 seed=1,uint32 k0=0,uint32 k1=100000000,uint32 k2=200000000,uint32 k3=300000000,uint32 k4=400000000,uint32 k5=500000000,uint32 k6=600000000,uint32 k7=700000000);
#endif // HAS_VEC8V

VMATH_INLINE Vec4V_out XorShift4V_LRL_32_13_17_5(Vec4V& state) // returns four random values in the range [0..1)
{
	__m128i temp = _mm_castps_si128(state);
	temp = _mm_xor_si128(temp,_mm_slli_epi32(temp,13));
	temp = _mm_xor_si128(temp,_mm_srli_epi32(temp,17));
	temp = _mm_xor_si128(temp,_mm_slli_epi32(temp,5));
	state = Vec4V(_mm_castsi128_ps(temp)); // store new state
	return GetFloatValueFromRandomBits(temp);
}

VMATH_INLINE Vec4V_out XorShift4V_RLR_31_11_13_20(Vec4V& state) // returns four random values in the range [0..1)
{
	__m128i temp = _mm_castps_si128(state);
	temp = _mm_xor_si128(temp,_mm_srli_epi32(temp,11));
	temp = _mm_xor_si128(temp,_mm_slli_epi32(temp,13));
	temp = _mm_andnot_si128(_mm_set1_epi32(0x80000000),temp);
	temp = _mm_xor_si128(temp,_mm_srli_epi32(temp,20));
	state = Vec4V(_mm_castsi128_ps(temp)); // store new state
	return GetFloatValueFromRandomBits(temp);
}

#if HAS_VEC8V
VMATH_INLINE Vec8V_out XorShift8V_LRL_32_13_17_5(Vec8V& state) // returns four random values in the range [0..1)
{
	__m256i temp = _mm256_castps_si256(state);
	temp = _mm256_xor_si256(temp,_mm256_slli_epi32(temp,13));
	temp = _mm256_xor_si256(temp,_mm256_srli_epi32(temp,17));
	temp = _mm256_xor_si256(temp,_mm256_slli_epi32(temp,5));
	state = Vec8V(_mm256_castsi256_ps(temp)); // store new state
	return GetFloatValueFromRandomBits(temp);
}

VMATH_INLINE Vec8V_out XorShift8V_RLR_31_11_13_20(Vec8V& state) // returns four random values in the range [0..1)
{
	__m256i temp = _mm256_castps_si256(state);
	temp = _mm256_xor_si256(temp,_mm256_srli_epi32(temp,11));
	temp = _mm256_xor_si256(temp,_mm256_slli_epi32(temp,13));
	temp = _mm256_andnot_si256(_mm256_set1_epi32(0x80000000),temp);
	temp = _mm256_xor_si256(temp,_mm256_srli_epi32(temp,20));
	state = Vec8V(_mm256_castsi256_ps(temp)); // store new state
	return GetFloatValueFromRandomBits(temp);
}
#endif // HAS_VEC8V

template <int N> class IntegerType
{
public:
	typedef void T_unsigned;
	typedef void T_signed;
};
template <> class IntegerType<(8<<0)> { public: typedef uint8  T_unsigned; typedef int8  T_signed; };
template <> class IntegerType<(8<<1)> { public: typedef uint16 T_unsigned; typedef int16 T_signed; };
template <> class IntegerType<(8<<2)> { public: typedef uint32 T_unsigned; typedef int32 T_signed; };
template <> class IntegerType<(8<<3)> { public: typedef uint64 T_unsigned; typedef int64 T_signed; };

template <typename T> class IntegerType_T
{
public:
	typedef void T_unsigned;
	typedef void T_signed;
};
template <> class IntegerType_T<uint8 > { public: typedef uint8  T_unsigned; typedef int8  T_signed; };
template <> class IntegerType_T<uint16> { public: typedef uint16 T_unsigned; typedef int16 T_signed; };
template <> class IntegerType_T<uint32> { public: typedef uint32 T_unsigned; typedef int32 T_signed; };
template <> class IntegerType_T<uint64> { public: typedef uint64 T_unsigned; typedef int64 T_signed; };
template <> class IntegerType_T< int8 > { public: typedef uint8  T_unsigned; typedef int8  T_signed; };
template <> class IntegerType_T< int16> { public: typedef uint16 T_unsigned; typedef int16 T_signed; };
template <> class IntegerType_T< int32> { public: typedef uint32 T_unsigned; typedef int32 T_signed; };
template <> class IntegerType_T< int64> { public: typedef uint64 T_unsigned; typedef int64 T_signed; };

template <typename T,int N,int a,int b,int c> VMATH_INLINE T XorShift_LRL(T x)
{
	static_assert(N <= sizeof(T)*8,"");
	x ^= (x << a); x &= BITMASK(T,N);
	x ^= (x >> b);
	x ^= (x << c); x &= BITMASK(T,N);
	return x;
}

template <typename T,int N,int a,int b,int c> VMATH_INLINE T XorShift_RLR(T x)
{
	static_assert(N <= sizeof(T)*8,"");
	x ^= (x >> a);
	x ^= (x << b); x &= BITMASK(T,N);
	x ^= (x >> c);
	return x;
}

template <typename T,int N,int a> VMATH_INLINE T XorShift_R_Inverse(T x)
{
	static_assert(N <= sizeof(T)*8,"");
	T y = x;
	for (int i = 0; i < N; i += a)
		y = x ^ (y >> a);
	return y;
}

template <typename T,int N,int a> VMATH_INLINE T XorShift_L_Inverse(T x)
{
	static_assert(N <= sizeof(T)*8,"");
	T y = x;
	for (int i = 0; i < N; i += a)
		y = x ^ (y << a);
	return y;
}

template <typename T,int N,int a,int b,int c> VMATH_INLINE T XorShift_LRL_Inverse(T x)
{
	static_assert(N <= sizeof(T)*8,"");
	x = XorShift_L_Inverse<T,N,c>(x); x &= BITMASK(T,N);
	x = XorShift_R_Inverse<T,N,b>(x);
	x = XorShift_L_Inverse<T,N,a>(x); x &= BITMASK(T,N);
	return x;
}

template <typename T,int N,int a,int b,int c> VMATH_INLINE T XorShift_RLR_Inverse(T x)
{
	static_assert(N <= sizeof(T)*8,"");
	x = XorShift_R_Inverse<T,N,c>(x);
	x = XorShift_L_Inverse<T,N,b>(x); x &= BITMASK(T,N);
	x = XorShift_R_Inverse<T,N,a>(x);
	return x;
}

VMATH_INLINE float XorShift_LRL_32_13_17_5(uint32& state)
{
	state = XorShift_LRL<uint32,32,13,17,5>(state);
	return GetFloatValueFromRandomBits(state);
}

VMATH_INLINE float XorShift_RLR_31_11_13_20(uint32& state)
{
	state = XorShift_RLR<uint32,31,11,13,20>(state);
	return GetFloatValueFromRandomBits(state);
}

uint32 XorShift_LRL_32_13_17_5_SkipAhead(uint32 x,uint32 k);
uint32 XorShift_RLR_31_11_13_20_SkipAhead(uint32 x,uint32 k);

uint32 FindXorShiftsUsingExactlyNBits(uint32 N);

float GetRandomValue(uint32* state=nullptr); // [0..1)
float GetRandomValueInRange(float minVal,float maxVal,uint32* state=nullptr);

Vec2V_out GetRandomPoint2D(Vec2V_arg bmin,Vec2V_arg bmax,uint32* state=nullptr);
Vec3V_out GetRandomPoint3D(Vec3V_arg bmin,Vec3V_arg bmax,uint32* state=nullptr);
Vec3V_out GetRandomDirection3D(uint32* state=nullptr);
Mat33V_out GetRandomRotation3D(uint32* state=nullptr);

// http://c42f.github.io/2015/09/21/inverting-32-bit-wang-hash.html
VMATH_INLINE uint32 wang_hash(uint32 key)
{
    const uint32 k0 = key;
    const uint32 k1 = k0 + ~(k0 << 15);
    const uint32 k2 = k1 ^  (k1 >> 10);
    const uint32 k3 = k2 +  (k2 <<  3);
    const uint32 k4 = k3 ^  (k3 >>  6);
    const uint32 k5 = k4 + ~(k4 << 11);
    const uint32 k6 = k5 ^  (k5 >> 16);
    return k6;
}

VMATH_INLINE uint32 wang_hash_inv(uint32 hashval)
{
    const uint32 k6 = hashval;
    const uint32 k5 = k6 ^ (k6 >> 16);
    const uint32 k4 = 4290770943UL * ~k5;
    const uint32 k3 = k4 ^ (k4 >> 6) ^ (k4 >> 12) ^ (k4 >> 18) ^ (k4 >> 24) ^ (k4 >> 30);
    const uint32 k2 = 954437177UL * k3;
    const uint32 k1 = k2 ^ (k2 >> 10) ^ (k2 >> 20) ^ (k2 >> 30);
    const uint32 k0 = 3221192703UL * ~k1;
    return k0;
}

// https://naml.us/post/inverse-of-a-hash-function/
// https://gist.github.com/lh3/974ced188be2f90422cc
VMATH_INLINE uint64 wang_hash64(uint64 key)
{
	key = (~key) + (key << 21); // key = (key << 21) - key - 1;
	key = key ^ (key >> 24);
	key = (key + (key << 3)) + (key << 8); // key * 265
	key = key ^ (key >> 14);
	key = (key + (key << 2)) + (key << 4); // key * 21
	key = key ^ (key >> 28);
	key = key + (key << 31);
	return key;
}

VMATH_INLINE uint64 wang_hash64_inv(uint64 val)
{
	uint64 tmp;
	
	// Invert val = val + (val << 31)
	tmp = val - (val << 31);
	val = val - (tmp << 31);
	
	// Invert val = val ^ (val >> 28)
	tmp = val ^ (val >> 28);
	val = val ^ (tmp >> 28);
	
	// Invert val *= 21
	val *= 14933078535860113213ULL;
	
	// Invert val = val ^ (val >> 14)
	tmp = val ^ (val >> 14);
	tmp = val ^ (tmp >> 14);
	tmp = val ^ (tmp >> 14);
	val = val ^ (tmp >> 14);
	
	// Invert val *= 265
	val *= 15244667743933553977ULL;
	
	// Invert val = val ^ (val >> 24)
	tmp = val ^ (val >> 24);
	val = val ^ (tmp >> 24);
	
	// Invert val = (~val) + (val << 21)
	tmp = ~val;
	tmp = ~(val - (tmp << 21));
	tmp = ~(val - (tmp << 21));
	val = ~(val - (tmp << 21));
	
	return val;
}

// http://cliodhna.cop.uop.edu/~hetrick/na_faq.html
VMATH_INLINE uint64 MWC(uint64 x)
{
	const uint32 a = (uint32)(x);
	const uint32 b = (uint32)(x >> 32);
	return 0x5CDCFAA7ULL*a + b;
}

#endif // _INCLUDE_VMATH_RANDOM_H_

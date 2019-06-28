// ===========================
// common/vmath/vmath_common.h
// ===========================

#ifndef _INCLUDE_VMATH_COMMON_H_
#define _INCLUDE_VMATH_COMMON_H_

// nice guide for SSE/AVX intrinsics ..
// https://software.intel.com/sites/landingpage/IntrinsicsGuide/

#include "common/common.h"
#if !defined(_OFFLINETOOL)
#include "common/types.h"
#endif // !defined(_OFFLINETOOL)

#if PLATFORM_PC && defined(__AVX2__) && !defined(__VMATH_FMA__) // let's use FMA on PC's with AVX2
#define __VMATH_FMA__
#endif

#if PLATFORM_PS4 && !defined(__AVX__) // PS4 has AVX, might as well enable it even if it's slow
#define __AVX__
#endif

#define NO_AVX_OR_FMA (0) // switch this on to completely disable AVX,AVX2,FMA etc.

#if NO_AVX_OR_FMA
#ifdef __AVX__
#undef __AVX__
#endif
#ifdef __AVX2__
#undef __AVX2__
#endif
#ifdef __VMATH_FMA__
#undef __VMATH_FMA__
#endif
#endif

#if defined(__AVX__) && PLATFORM_PC // technically PS4 has AVX, but it's slow
#define HAS_VEC8V (1)
#else
#define HAS_VEC8V (0)
#endif
#if HAS_VEC8V
#define HAS_VEC8V_ONLY(...) __VA_ARGS__
#else
#define HAS_VEC8V_ONLY(...)
#endif

#define HAS_VEC16V (0 && HAS_VEC8V) // requires AVX512, which seems to be supported in VS2017 but will not run on my machines
#if HAS_VEC16V
#define HAS_VEC16V_ONLY(...) __VA_ARGS__
#else
#define HAS_VEC16V_ONLY(...)
#endif

#if 1 && defined(__AVX2__)
#define VMATH_USE_GATHER_SCATTER
#endif

#if PLATFORM_PC
#define VMATH_INLINE __forceinline
#elif PLATFORM_PS4
#define VMATH_INLINE inline __attribute__((always_inline))
#endif

#define PRINTV(var) PrintV(#var,var)

#define VMATH_INCLUDING_FROM_VMATH_COMMON
#include "vmath_pc.h"
#include "vmath_platform_sse.h"
#undef VMATH_INCLUDING_FROM_VMATH_COMMON

#define VEC2_ARGS(v) (v).x,(v).y
#define VEC3_ARGS(v) (v).x,(v).y,(v).z
#define VEC4_ARGS(v) (v).x,(v).y,(v).z,(v).w

#define ARRAY2_ARGS(v) (v)[0],(v)[1]
#define ARRAY3_ARGS(v) (v)[0],(v)[1],(v)[2]
#define ARRAY4_ARGS(v) (v)[0],(v)[1],(v)[2],(v)[3]
#define ARRAY5_ARGS(v) (v)[0],(v)[1],(v)[2],(v)[3],(v)[4]
#define ARRAY6_ARGS(v) (v)[0],(v)[1],(v)[2],(v)[3],(v)[4],(v)[5]
#define ARRAY7_ARGS(v) (v)[0],(v)[1],(v)[2],(v)[3],(v)[4],(v)[5],(v)[6]
#define ARRAY8_ARGS(v) (v)[0],(v)[1],(v)[2],(v)[3],(v)[4],(v)[5],(v)[6],(v)[7]
#define ARRAY9_ARGS(v) (v)[0],(v)[1],(v)[2],(v)[3],(v)[4],(v)[5],(v)[6],(v)[7],(v)[8]

enum VectorConstantInitializer
{
	V_ZERO,
	V_ONE,
	V_XAXIS,
	V_YAXIS,
	V_ZAXIS,
	V_WAXIS,
};

#if !defined(__AVX2__)
// https://codingforspeed.com/counting-the-number-of-leading-zeros-for-a-32-bit-integer-signed-or-unsigned/
template <typename T> VMATH_INLINE unsigned leading_zero_naive(T x)
{
	if (x == 0)
		return sizeof(T)*8;
	else {
		const T msb = T(1) << (sizeof(T)*8 - 1);
		StaticAssert(msb > 0);
		unsigned count = 0;
		while ((x & msb) == 0) {
			count++;
			x <<= 1;
		}
		return count;
	}
}
#endif // !defined(__AVX2__)

VMATH_INLINE uint32 Log2CeilingInt(uint32 num)
{
	if (num > 0)
	#if defined(__AVX2__)
		return 32 - _lzcnt_u32(num - 1); // AVX2
	#else
		return 32 - leading_zero_naive<uint32>(num - 1);
	#endif
	else
		return 0;
}

VMATH_INLINE uint32 Log2FloorInt(uint32 num)
{
	return Log2CeilingInt(num + 1) - 1;
}

VMATH_INLINE uint32 RoundUpToPow2(uint32 num)
{
	return BIT(Log2CeilingInt(num));
}

VMATH_INLINE uint32 RoundDownToPow2(uint32 num)
{
	return RoundUpToPow2(num/2 + 1);
}

#endif // _INCLUDE_VMATH_COMMON_H_

// =======================
// common/vmath/vmath_pc.h
// =======================

#ifndef _INCLUDE_VMATH_PC_H_
#define _INCLUDE_VMATH_PC_H_

#ifndef VMATH_INCLUDING_FROM_VMATH_COMMON
#error "include vmath_common.h instead of this header!"
#endif

#if PLATFORM_PC
#include <immintrin.h>
#include <math.h>
#elif PLATFORM_PS4
#include <assert.h>
#include "common/types.h"
#endif 

#endif // _INCLUDE_VMATH_PC_H_

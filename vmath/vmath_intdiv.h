// ===========================
// common/vmath/vmath_intdiv.h
// ===========================

#ifndef _INCLUDE_VMATH_INTDIV_H_
#define _INCLUDE_VMATH_INTDIV_H_

#include "vmath_common.h"

uint32 IntegerDiv_GetShift(uint32 q, uint32 p_bits = 32);
uint32 IntegerDiv_GetMultiplier(uint32 q, uint32& s, bool optimize = true);
uint32 IntegerDiv_GetMultiplierForFixedShift(uint32 q, uint32 fixedShift = 32, uint32 check_p_max = 0);

#endif // _INCLUDE_VMATH_INTDIV_H_
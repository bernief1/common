// =============================
// common/vmath/vmath_intdiv.cpp
// =============================

#include "vmath_intdiv.h"

uint32 IntegerDiv_GetShift(uint32 q, uint32 p_bits)
{
	// p_bits specifies the maximum number of bits allowed for the numerator
	ForceAssert(p_bits <= 32);
	uint32 s = Log2CeilingInt(q);
	if (q & (q - 1))
		s += p_bits - 1;
	return s;
}

uint32 IntegerDiv_GetMultiplier(uint32 q, uint32& s, bool optimize)
{
	uint32 m = 1;
	if (optimize && (q == 3 || q == 5 || q == 6))
		s--; // this seems to be ok for q < 7 (and a lot of other values of q such as 17 - but i haven't found the pattern)
	m = ((1ULL << s) + q - 1)/q;
	if (optimize) {
		while ((m&1) == 0) {
			m >>= 1;
			s--;
		}
	}
	return m;
}

uint32 IntegerDiv_GetMultiplierForFixedShift(uint32 q, uint32 fixedShift, uint32 check_p_max)
{
	ForceAssert(q > 2);
	const uint32 m = IntegerDiv_GetMultiplier(q, fixedShift, false);
	if (check_p_max != 0) {
		// if this multiplier will fail for some numerator up to 'check_p_max', then it will
		// fail for some numerator in the range [check_p_max - q .. check_p_max] .. i think.
		bool failed = false;
		for (uint64 p = check_p_max; p >= (uint64)check_p_max - q; p--) {
			const uint32 r = (uint32)((p*m) >> fixedShift);
			ForceAssertf(r == p/q, "%u/%u -> %u, expected %u! (m=%u, s=%u)", (uint32)p, q, r, (uint32)p/q, m, fixedShift);
		}
	}
	return m;
}
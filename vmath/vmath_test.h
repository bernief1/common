// =========================
// common/vmath/vmath_test.h
// =========================

#ifndef _INCLUDE_VMATH_TEST_H_
#define _INCLUDE_VMATH_TEST_H_

#include "vmath_common.h"

#if (1 || defined(_DEBUG)) && PLATFORM_PC && defined(_OFFLINETOOL)
#define VMATH_TEST (1)
#else
#define VMATH_TEST (0)
#endif

#define VMATH_TEST_LOAD_STORE        (1 && VMATH_TEST)
#define VMATH_TEST_FIXED_CONVERSIONS (1 && VMATH_TEST)
#define VMATH_TEST_GEOM              (1 && VMATH_TEST)
#define VMATH_TEST_RANDOM            (1 && VMATH_TEST)
#define VMATH_TEST_TRANSCENDENTAL    (1 && VMATH_TEST)
#define VMATH_TEST_INTEGER_DIV       (1 && VMATH_TEST)
#define VMATH_TEST_HALTON_SEQ        (1 && VMATH_TEST)

#if VMATH_TEST_LOAD_STORE
void TestLoadStore();
void TestLoadStoreFloat16();
void TestLoadStoreFixed();
#endif // VMATH_TEST_LOAD_STORE

#if VMATH_TEST_FIXED_CONVERSIONS
void TestSignedNormConversions();
#endif // VMATH_TEST_FIXED_CONVERSIONS

#if VMATH_TEST_GEOM
void TestBuildSphereThroughPoints();
void TestIntersectionCode();
#endif // VMATH_TEST_GEOM

#if VMATH_TEST_RANDOM
void TestXorShift();
void TestRandom();
#endif // VMATH_TEST_RANDOM

#if VMATH_TEST_TRANSCENDENTAL
void TestExp2Performance();
void TestLog2Performance();
void TestSinPIPerformance();
void TestSinCos();
#endif // VMATH_TEST_TRANSCENDENTAL

#if VMATH_TEST_INTEGER_DIV
bool TestIntegerDiv(uint32 q_min = 2,uint32 q_max = 64,uint32 q = 0,uint32 p_bits = 32,uint32 fixedShift = 0,bool silent = false);
#endif // VMATH_TEST_INTEGER_DIV

#if VMATH_TEST_HALTON_SEQ
void TestHaltonSequenceGenerator();
#endif // VMATH_TEST_HALTON_SEQ

#endif // _INCLUDE_VMATH_TEST_H_
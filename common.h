// ===============
// common/common.h
// ===============

#ifndef _INCLUDE_COMMON_COMMON_H_
#define _INCLUDE_COMMON_COMMON_H_

#ifndef COMMONDLL_API
#define COMMONDLL_API // TODO XXX -- move this to files which specifically included common/common.h, or get rid of it altogether
#endif

// https://stackoverflow.com/questions/70013/how-to-detect-if-im-compiling-code-with-visual-studio-2008
// https://sourceforge.net/p/predef/wiki/Compilers/#microsoft-visual-c
#if (_MSC_VER >= 1910)
#define VISUAL_STUDIO_VERSION 2017 // MSVC++ 14.1
#define VISUAL_STUDIO_VERSION_STR "2017"
#elif (_MSC_VER == 1900)
#define VISUAL_STUDIO_VERSION 2015 // MSVC++ 14.0
#define VISUAL_STUDIO_VERSION_STR "2015"
#elif (_MSC_VER == 1800)
#define VISUAL_STUDIO_VERSION 2013 // MSVC++ 12.0
#define VISUAL_STUDIO_VERSION_STR "2013"
#elif (_MSC_VER == 1700)
#define VISUAL_STUDIO_VERSION 2012 // MSVC++ 11.0
#define VISUAL_STUDIO_VERSION_STR "2012"
#elif (_MSC_VER == 1600)
#define VISUAL_STUDIO_VERSION 2010 // MSVC++ 10.0
#define VISUAL_STUDIO_VERSION_STR "2010"
#elif (_MSC_VER == 1500)
#define VISUAL_STUDIO_VERSION 2008 // MSVC++ 9.0
#define VISUAL_STUDIO_VERSION_STR "2008"
#elif (_MSC_VER == 1400)
#define VISUAL_STUDIO_VERSION 2005 // MSVC++ 8.0
#define VISUAL_STUDIO_VERSION_STR "2005"
#else
#define VISUAL_STUDIO_VERSION 0 // unknown
#define VISUAL_STUDIO_VERSION_STR "_unknown"
#endif

#if defined(SN_TARGET_PS4) // this is how we determine platform ..
#define PLATFORM_PS4 (1)
#define PLATFORM_PC (0)
#define PS4_ONLY(...) __VA_ARGS__
#define PC_ONLY(...)
#define XXX_GAME (1)
#elif defined(_WIN64) || defined(_WIN32)
#define PLATFORM_PS4 (0)
#define PLATFORM_PC (1)
#define PS4_ONLY(...)
#define PC_ONLY(...) __VA_ARGS__
#if defined(_OFFLINETOOL)
#define XXX_GAME (0)
#pragma warning(error:4477) // 'function' : format string 'string' requires an argument of type 'type', but variadic argument number has type 'type'
#pragma warning(disable:4996) //  'stricmp': The POSIX name for this item is deprecated. Instead, use the ISO C and C++ conformant name: _stricmp.
#else
#define XXX_GAME (1)
#endif
#endif // defined(_WIN64) || defined(_WIN32)

#if PLATFORM_PC && (defined(_OFFLINETOOL) || defined(_GAMETOOL))
#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX
#include <windows.h>
#include <assert.h>
#endif // PLATFORM_PC && (defined(_OFFLINETOOL) || defined(_GAMETOOL))
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <algorithm>
#include <map>
#include <vector>
#if XXX_GAME
#include "Debug/Debug_Assert.h"
#include <type_traits>
#endif // XXX_GAME

#if defined(_OPENGL)
#if defined(_WIN64)
// NOTE: need to copy freeglut.dll to C:/Windows or somewhere in your paths
#define FREEGLUT_LIB_PRAGMAS (0) // because glut/freeglut/include/GL/freeglut_std.h tries to pull in freeglut.lib without a path ..
#define FREEGLUT_INCLUDE_GLEW_2_1_0 (1) // https://sourceforge.net/projects/glew/
#define GLEW_STATIC (1) // otherwise, need to put the dll somewhere ..
#if !FREEGLUT_INCLUDE_GLEW_2_1_0
#include "GraphicsTools/external/glut/freeglut/include/GL/glut.h"
#include "GraphicsTools/external/glut/freeglut/include/GL/glext.h"
#endif // !FREEGLUT_INCLUDE_GLEW_2_1_0
#include "GraphicsTools/external/glut/freeglut/include/GL/freeglut.h" // NOTE: i had to add this to freeglut_std.h: "#include "../../../../glew-2.1.0/include/GL/glew.h"
#pragma comment(lib,"GraphicsTools/external/glut/freeglut/lib/x64/freeglut.lib")
#if FREEGLUT_INCLUDE_GLEW_2_1_0
#if defined(GLEW_STATIC)
#pragma comment(lib,"GraphicsTools/external/glew-2.1.0/lib/Release/x64/glew32s.lib")
#else
#pragma comment(lib,"GraphicsTools/external/glew-2.1.0/lib/Release/x64/glew32.lib")
#endif
#endif // FREEGLUT_INCLUDE_GLEW_2_1_0
#else
//#pragma comment(lib,"GraphicsTools/external/glut/freeglut/lib/freeglut.lib") // doesn't run .. use the standard glut (which doesn't run in 64-bit!)
#define GLUT_NO_LIB_PRAGMA
#include "GraphicsTools/external/glut/glut.h"
#pragma comment(lib,"GraphicsTools/external/glut/glut32.lib")
#endif
// other stuff glut tries to link .. not sure which are necessary
#pragma comment(lib,"glu32.lib")    // link OpenGL Utility lib
#pragma comment(lib,"opengl32.lib") // link Microsoft OpenGL lib
#pragma comment(lib,"gdi32.lib")    // link Windows GDI lib
#pragma comment(lib,"winmm.lib")    // link Windows MultiMedia lib
#pragma comment(lib,"user32.lib")   // link Windows user lib
#endif // defined(_OPENGL)

// https://stackoverflow.com/questions/4415530/equivalents-to-msvcs-countof-in-other-compilers
template <typename T,size_t N> inline constexpr size_t countof(T(&arr)[N]) { return std::extent<T[N]>::value; }
template <typename T,size_t N> inline constexpr int icountof(T(&arr)[N]) { return static_cast<int>(std::extent<T[N]>::value); }

#ifndef BIT
#define BIT(index) (1U<<(index))
#endif
#ifndef BIT64
#define BIT64(index) (1ULL<<(index))
#endif
#ifndef PI
#define PI 3.141592653589793238462643383f
#endif
#ifndef DEG_TO_RAD
#define DEG_TO_RAD (PI/180.0f)
#endif
#ifndef RAD_TO_DEG
#define RAD_TO_DEG (180.0f/PI)
#endif

#define GET_STR_VARARGS(str,size,format) \
	char str[size] = ""; \
	va_list args; \
	va_start(args, format); \
	vsnprintf(str, size, format, args); \
	va_end(args)

#if defined(_OFFLINETOOL)
#if 0 || defined(_DEBUG) // <-- change to 1 to force all asserts in release
#define USE_ASSERT (1)
#else
#define USE_ASSERT (0)
#endif
inline bool InternalAssert(bool cond, const char* file, const char* func, int line, const char* code, const char* format = nullptr, ...)
{
	if (!cond) {
		fprintf(stderr, "\nASSERT in %s, function %s, line %d: %s\n", file, func, line, code);
		if (format) {
			GET_STR_VARARGS(msg, 1024, format);
			fprintf(stderr, "%s\n", msg);
		}
		__debugbreak();
		system("pause");
	}
	return cond;
}
#define ForceAssert(cond)             (void)InternalAssert(!!(cond),__FILE__,__FUNCTION__,__LINE__,#cond)
#define ForceAssertf(cond,format,...) (void)InternalAssert(!!(cond),__FILE__,__FUNCTION__,__LINE__,#cond,format,__VA_ARGS__)
#define ForceAssertMsg(cond,msg)      (void)InternalAssert(!!(cond),__FILE__,__FUNCTION__,__LINE__,#cond,"%s",msg)
#define ForceAssertVerify(cond)             InternalAssert(!!(cond),__FILE__,__FUNCTION__,__LINE__,#cond)
#define ForceAssertVerifyf(cond,format,...) InternalAssert(!!(cond),__FILE__,__FUNCTION__,__LINE__,#cond,format,__VA_ARGS__)
#define ForceAssertVerifyMsg(cond,msg)      InternalAssert(!!(cond),__FILE__,__FUNCTION__,__LINE__,#cond,"%s",msg)
#if USE_ASSERT
#define DEBUG_ASSERT             ForceAssert
#define DEBUG_ASSERT_FMT         ForceAssertf
#define DEBUG_ASSERT_STR         ForceAssertMsg
#define DEBUG_ASSERT_VERIFY      ForceAssertVerify
#define DEBUG_ASSERT_VERIFY_FMT  ForceAssertVerifyf
#define DEBUG_ASSERT_VERIFY_STR  ForceAssertVerifyMsg
#else
#define DEBUG_ASSERT(cond)
#define DEBUG_ASSERT_FMT(cond,format,...)
#define DEBUG_ASSERT_STR(cond,msg)
#define DEBUG_ASSERT_VERIFY(cond)                (cond)
#define DEBUG_ASSERT_VERIFY_FMT(cond,format,...) (cond)
#define DEBUG_ASSERT_VERIFY_STR(cond,msg)        (cond)
#endif
#elif XXX_GAME
#define USE_ASSERT (!(RELEASE_BURN || DEBUG_ASSERT_DISABLE))
#if USE_ASSERT
// TODO XXX - implement DEBUG_ASSERT_VERIFY in Debug/Debug_Assert.h
inline bool InternalAssert(bool cond, const char* file, const char* func, int line, const char* code, const char* format = nullptr, ...)
{
	if (!cond) {
		char temp[1024];
		sprintf(temp, "ASSERT in %s, function %s, line %d: %s", file, func, line, code);
		if (format) {
			GET_STR_VARARGS(msg, 1024, format);
			strcat(temp, "\n");
			strcat(temp, msg);
		}
		DEBUG_ASSERT_STR(false, temp);
	}
	return cond;
}
#define DEBUG_ASSERT_VERIFY(cond)                InternalAssert(!!(cond),__FILE__,__FUNCTION__,__LINE__,#cond)
#define DEBUG_ASSERT_VERIFY_FMT(cond,format,...) InternalAssert(!!(cond),__FILE__,__FUNCTION__,__LINE__,#cond,format,__VA_ARGS__)
#define DEBUG_ASSERT_VERIFY_STR(cond,msg)        InternalAssert(!!(cond),__FILE__,__FUNCTION__,__LINE__,#cond,"%s",msg)
#else
#define DEBUG_ASSERT_VERIFY(cond)                (cond)
#define DEBUG_ASSERT_VERIFY_FMT(cond,format,...) (cond)
#define DEBUG_ASSERT_VERIFY_STR(cond,msg)        (cond)
#endif
#define ForceAssert          DEBUG_ASSERT // all asserts in XXX_GAME are active in non-final builds - DON'T USE THESE MACROS in XXX code
#define ForceAssertf         DEBUG_ASSERT_FMT
#define ForceAssertMsg       DEBUG_ASSERT_STR
#define ForceAssertVerify    DEBUG_ASSERT_VERIFY
#define ForceAssertVerifyf   DEBUG_ASSERT_VERIFY_FMT
#define ForceAssertVerifyMsg DEBUG_ASSERT_VERIFY_STR
#endif // XXX_GAME
#define StaticAssert(cond)        static_assert(cond,"")
#define StaticAssertMsg(cond,msg) static_assert(cond,msg)

#if USE_ASSERT
#define ASSERT_ONLY(...) __VA_ARGS__
#else
#define ASSERT_ONLY(...)
#endif

#if defined(_OFFLINETOOL)
// basic integer types
typedef unsigned char      uint8;
typedef unsigned short     uint16;
typedef unsigned int       uint32;
typedef unsigned long long uint64;
typedef signed   char      int8;
typedef signed   short     int16;
typedef signed   int       int32;
typedef signed   long long int64;

StaticAssert(sizeof(uint8 ) == 1 && sizeof(int8 ) == 1);
StaticAssert(sizeof(uint16) == 2 && sizeof(int16) == 2);
StaticAssert(sizeof(uint32) == 4 && sizeof(int32) == 4);
StaticAssert(sizeof(uint64) == 8 && sizeof(int64) == 8);
#endif // defined(_OFFLINETOOL)

#if XXX_GAME
#include "Math/vec_common.h"
#else
template <typename T> inline T Min(T a, T b) { return b < a ? b : a; }
template <typename T> inline T Min(T a, T b, T c) { return Min(Min(a, b), c); }
template <typename T> inline T Min(T a, T b, T c, T d) { return Min(Min(a, b), Min(c, d)); }
template <typename T> inline T Max(T a, T b) { return a < b ? b : a; }
template <typename T> inline T Max(T a, T b, T c) { return Max(Max(a, b), c); }
template <typename T> inline T Max(T a, T b, T c, T d) { return Max(Max(a, b), Max(c, d)); }

template <typename T,typename T2,typename T3> inline T Clamp(T a, T2 minVal, T3 maxVal) { return Max((T)minVal, Min(a, (T)maxVal)); }

inline int8   Abs(int8   a) { return a >= 0    ? a : -a; } // only defined for signed types
inline int16  Abs(int16  a) { return a >= 0    ? a : -a; }
inline int32  Abs(int32  a) { return a >= 0    ? a : -a; }
inline int64  Abs(int64  a) { return a >= 0    ? a : -a; }
inline float  Abs(float  a) { return a >= 0.0f ? a : -a; }
inline double Abs(double a) { return a >= 0.0  ? a : -a; }

template <typename T> inline T Floor      (T a); // some functions are only defined for floats and doubles ..
template <typename T> inline T Ceiling    (T a);
template <typename T> inline T Truncate   (T a);
template <typename T> inline T Round      (T a);
template <typename T> inline T Frac       (T a);
template <typename T> inline T Saturate   (T a);
template <typename T> inline T Sqrt       (T a);
template <typename T> inline T Sqr        (T a) { return a*a; } // .. other functions are defined for all numeric types
template <typename T> inline T Recip      (T a);
template <typename T> inline T RecipSqrt  (T a);
template <typename T> inline T Sin        (T a);
template <typename T> inline T Cos        (T a);
template <typename T> inline T MultiplyAdd(T a, T b, T c) { return a*b + c; }
template <typename T> inline T MultiplySub(T a, T b, T c) { return a*b - c; }
template <typename T> inline T MultiplyAdd(T a, T b, T c, T d) { return a*b + c*d; }
template <typename T> inline T MultiplyAdd(T a, T b, T c, T d, T e) { return a*b + c*d + e; }
template <typename T> inline T MultiplyAdd(T a, T b, T c, T d, T e, T f) { return a*b + c*d + e*f; }

template <> inline float Floor    <float>(float a) { return floorf(a); }
template <> inline float Ceiling  <float>(float a) { return ceilf(a); }
template <> inline float Truncate <float>(float a) { return truncf(a); }
template <> inline float Round    <float>(float a) { return roundf(a); }
template <> inline float Frac     <float>(float a) { return a - floorf(a); }
template <> inline float Saturate <float>(float a) { return Clamp(a, 0.0f, 1.0f); }
template <> inline float Sqrt     <float>(float a) { return sqrtf(a); }
template <> inline float Recip    <float>(float a) { return 1.0f/a; }
template <> inline float RecipSqrt<float>(float a) { return 1.0f/sqrtf(a); } // or sqrtf(1.0f/a)
template <> inline float Sin      <float>(float a) { return sinf(a); }
template <> inline float Cos      <float>(float a) { return cosf(a); }

template <> inline double Floor    <double>(double a) { return floor(a); }
template <> inline double Ceiling  <double>(double a) { return ceil(a); }
template <> inline double Truncate <double>(double a) { return trunc(a); }
template <> inline double Round    <double>(double a) { return round(a); }
template <> inline double Frac     <double>(double a) { return a - floor(a); }
template <> inline double Saturate <double>(double a) { return Clamp(a, 0.0, 1.0); }
template <> inline double Sqrt     <double>(double a) { return sqrt(a); }
template <> inline double Recip    <double>(double a) { return 1.0/a; }
template <> inline double RecipSqrt<double>(double a) { return 1.0/sqrt(a); } // or sqrtf(1.0/a)
template <> inline double Sin      <double>(double a) { return sin(a); }
template <> inline double Cos      <double>(double a) { return cos(a); }
#endif

// https://locklessinc.com/articles/classifying_floats/
inline bool IsFinite(float  a) { return (reinterpret_cast<const uint16*>(&a)[1] & 0x7F80) != 0x7F80; } // returns true for +/-INF or NaN
inline bool IsFinite(double a) { return (reinterpret_cast<const uint16*>(&a)[3] & 0x7FFF) != 0x7FFF; }

inline bool IsNaN(float  a) { return !(a == a); }
inline bool IsNaN(double a) { return !(a == a); }

#define BITMASK(T,N) ( (((T(1) << ((N) - 1)) - T(1)) << 1) + T(1) ) // this works even when N is sizeof(T)*8 - 1

#endif // _INCLUDE_COMMON_COMMON_H_
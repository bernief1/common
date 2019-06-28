// ============================
// common/vmath/vmath_float16.h
// ============================

#ifndef _INCLUDE_VMATH_FLOAT16_H_
#define _INCLUDE_VMATH_FLOAT16_H_

#include "vmath_common.h"
#include "vmath_floatvec.h"
#include "vmath_scalar.h"
#include "vmath_vec4.h"

#define FLOAT16_ZERO    0x0000
#define FLOAT16_NEGZERO 0x8000
#define FLOAT16_ONE     0x3C00
#define FLOAT16_NEGONE  0xBC00
#define FLOAT16_TWO     0x4000
#define FLOAT16_HALF    0x3800
#define FLOAT16_INF     0x7C00

#if PLATFORM_PS4
VMATH_INLINE uint16 Float32toFloat16(float x) { return _cvtss_sh(x,0); }
VMATH_INLINE float Float16toFloat32(uint16 x) { return _cvtsh_ss(x); }
#else
VMATH_INLINE uint16 Float32toFloat16(float x) { uint16 temp; ScalarV(x).StoreFloat16(&temp); return temp; }
VMATH_INLINE float Float16toFloat32(uint16 x) { const ScalarV temp = ScalarV::LoadFloat16(&x); return temp.f(); }
#endif

VMATH_INLINE static uint16 Float32toFloat16_REFERENCE(float f)
{
	// note that due to rounding this does not always return the exact same result as the hardware float16 instruction VCVTPS2PH (_mm_cvtps_ph)
	// an example is:
	// input 0x3C23D70A (0.01f)
	// output 0x211F (0.010002136) (hardware)
	// output 0x211E (0.009994507) (reference)
	const uint32 i = *reinterpret_cast<const uint32*>(&f);
	const uint16 s = (uint16)( (i & 0x80000000) >> 16);
	const uint16 e = (uint16)(((i & 0x7F800000) >> 23) - (127 - 15));
	const uint16 m = (uint16)( (i & 0x007FFFFF) >> 13);
	if (e <= 0) return s;
	else if (e > 30) return s | FLOAT16_INF;
	else return s | (e << 10) | m;
}

VMATH_INLINE static float Float16toFloat32_REFERENCE(uint16 h)
{
	// shift h to line up sign bit, then signed shift down to put mantissa and exponent in correct place, copying sign bit; mask out copied sign bit from exponent
	uint32 i = ((int32)((uint32)h << 16) >> 3) & ~0x70000000;
	// unless x is +/- 0.0f, add (127 - 15) to exponent
	if (h & 0x7FFF)
		i += 0x38000000;
	return *reinterpret_cast<const float*>(&i);
}

VMATH_INLINE uint16 Float32toFixed16_REFERENCE(float f) { return (uint16)(0.5f + 65535.0f*f); }
VMATH_INLINE float Fixed16toFloat32_REFERENCE(uint16 x) { return (float)x/65535.0f; }
VMATH_INLINE uint8 Float32toFixed8_REFERENCE(float f) { return (uint8)(0.5f + 255.0f*f); }
VMATH_INLINE float Fixed8toFloat32_REFERENCE(uint8 x) { return (float)x/255.0f; }

template <typename UnsignedIntType,unsigned bits> VMATH_INLINE UnsignedIntType Float32toFixed(float value)
{
	return (UnsignedIntType)(0.5f + value*(float)BITMASK(UnsignedIntType,bits));
}

template <typename UnsignedIntType,unsigned bits> VMATH_INLINE float FixedToFloat32(UnsignedIntType value)
{
	return (float)value/(float)BITMASK(UnsignedIntType,bits);
}

template <typename SignedIntType,unsigned bits> VMATH_INLINE SignedIntType Float32toFixedSigned(float value)
{
	return (SignedIntType)(0.5f + (0.5f + 0.5f*value)*(float)(BITMASK(SignedIntType,bits) - 1)) - BITMASK(SignedIntType,bits - 1);
}

template <typename SignedIntType,unsigned bits> VMATH_INLINE float FixedSignedToFloat32(SignedIntType value)
{
	return Max(-1.0f, (float)value/(float)BITMASK(SignedIntType,bits - 1));
}

template <typename T,unsigned manBits,unsigned expBits> VMATH_INLINE T Float32toSmallFloatUnsigned(float value)
{
	StaticAssert(sizeof(T)*8 >= manBits + expBits);
	const uint32 manBits_float32 = 23;
	const uint32 expBits_float32 = 8;
	const int expBias_float32 = 127;
	if (value <= 0.0f)
		return 0; // clamp to zero
	else {
		const uint32 i = *reinterpret_cast<const uint32*>(&value);
		const int expInf = (int)BITMASK(uint32,expBits); // exponent which represents INF
		const int expBias = (int)BITMASK(uint32,expBits - 1);
		const int e = (int)((i >> manBits_float32) & BITMASK(uint32,expBits_float32)) - expBias_float32 + expBias;
		if (e <= 0)
			return 0;
		else if (e >= expInf)
			return T(expInf << manBits);
		else {
			const T m = T((i & BITMASK(uint32,manBits_float32)) >> (manBits_float32 - manBits));
			return (T(e) << manBits) | m;
		}
	}
}

template <typename T,unsigned manBits,unsigned expBits> VMATH_INLINE T Float32toSmallFloat(float value)
{
	StaticAssert(sizeof(T)*8 >= 1 + manBits + expBits);
	const uint32 manBits_float32 = 23;
	const uint32 expBits_float32 = 8;
	const int expBias_float32 = 127;
	const uint32 i = *reinterpret_cast<const uint32*>(&value);
	const T s = T((i & 0x80000000) >> (32 - manBits - expBits));
	const int expInf = (int)BITMASK(uint32,expBits); // exponent which represents INF
	const int expBias = (int)BITMASK(uint32,expBits - 1);
	const int e = (int)((i >> manBits_float32) & BITMASK(uint32,expBits_float32)) - expBias_float32 + expBias;
	if (e <= 0)
		return s;
	else if (e >= expInf)
		return s | T(expInf << manBits);
	else {
		const T m = T((i & BITMASK(uint32,manBits_float32)) >> (manBits_float32 - manBits));
		return s | (T(e) << manBits) | m;
	}
}

class Float16
{
public:
	inline Float16() {}
	inline Float16(float f) : m_data(Float32toFloat16(f)) {}
	inline operator float() const { return Float16toFloat32(m_data); } 
	uint16 m_data;
};

class Pixel64RGBAFloat16
{
public:
	inline Pixel64RGBAFloat16() {}
	inline Pixel64RGBAFloat16(float r,float g,float b,float a = 1.0f) { Vec4V(r,g,b,1.0f).StoreFloat16V((uint16*)this); }
	inline Pixel64RGBAFloat16(Vec4V_arg rgba) { rgba.StoreFloat16V((uint16*)this); }
	inline Pixel64RGBAFloat16(Vec3V_arg rgb) { Vec4V(rgb,1.0f).StoreFloat16V((uint16*)this); }
	inline operator Vec4V() const { return Vec4V::LoadFloat16V((const uint16*)this); }
	inline operator Vec3V() const { return operator Vec4V().xyz(); }
	uint16 m_r, m_g, m_b, m_a;
};

#endif // _INCLUDE_VMATH_FLOAT16_H_

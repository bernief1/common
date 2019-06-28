// ====================
// common/vmath/vmath.h
// ====================

#ifndef _INCLUDE_VMATH_H_
#define _INCLUDE_VMATH_H_

#include "vmath_common.h"

// ================================================================================================

#if 0
template <typename T,unsigned m,unsigned e,int exp_bias> class SignedFloatType
{
public:
	enum { S = 1, M = m, E = e, BIAS = exp_bias };

	SignedFloatType() {}
	SignedFloatType(float value);
	operator float() const;

	union {
		struct {
			T m_sign:1;
			T m_mantissa:m;
			T m_exponent:e;
		};
		struct {
			T m_bits;
		};
	};
};

typedef SignedFloatType<uint32,23,8,-127> Float32Type;

template <typename T,unsigned m,unsigned e,int exp_bias> SignedFloatType<T,m,e,exp_bias>::SignedFloatType(float value)
{
	const Float32Type temp = reinterpret_cast<const Float32Type&>(value);
	m_sign = temp.m_sign;
	m_mantissa = (M > Float32Type::M) ? (((T)temp.m_mantissa) << (M - Float32Type::M)) : ((T)(temp.m_mantissa >> (Float32Type::M - M)));
	m_exponent = (T)((int)temp.m_exponent - BIAS + Float32Type::BIAS);
}

template <typename T,unsigned m,unsigned e,int exp_bias> SignedFloatType<T,m,e,exp_bias>::operator float() const
{
	Float32Type temp;
	temp.m_sign = m_sign;
	temp.m_mantissa = (Float32Type::M > M) ? (((uint32)m_mantissa) << (Float32Type::M - M)) : ((uint32)(m_mantissa >> (M - Float32Type::M)));
	temp.m_exponent = (T)((int)m_exponent - Float32Type::BIAS + BIAS);
	return reinterpret_cast<const float&>(temp);
}

template <typename T,unsigned m,unsigned e,int exp_bias> class UnsignedFloatType
{
public:
	enum { S = 0, M = m, E = e, BIAS = exp_bias };

	UnsignedFloatType() {}
	UnsignedFloatType(float value)
	{
		if (value > 0.0f) {
			const Float32Type temp = reinterpret_cast<const Float32Type&>(value);
			m_mantissa = (M > Float32Type::M) ? (((T)temp.m_mantissa) << (M - Float32Type::M)) : ((T)(temp.m_mantissa >> (Float32Type::M - M)));
			m_exponent = (T)((int)temp.m_exponent - BIAS + Float32Type::BIAS);
		} else
			m_bits = 0;
	}

	operator float() const
	{
		Float32Type temp;
		temp.m_sign = 0;
		temp.m_mantissa = (Float32Type::M > M) ? (((uint32)m_mantissa) << (Float32Type::M - M)) : ((uint32)(m_mantissa >> (M - Float32Type::M)););
		temp.m_exponent = (T)((int)m_exponent - Float32Type::BIAS + BIAS);
		return reinterpret_cast<const float&>(temp);
	}

	union {
		struct {
			T m_mantissa:m;
			T m_exponent:e;
		};
		struct {
			T m_bits;
		};
	};
};

inline void TestFloatTypes()
{
	for (int i = -100; i <= 100; i++) {
		const float f = (float)i/10.0f;
		const float f1 = SignedFloatType<uint16,10,5,-15>(f);
		printf("%f -> %f\n", f, f1);
	}
	system("pause");
	exit(0);
}
#endif

#endif // _INCLUDE_VMATH_H_
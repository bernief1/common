// ==============
// common/color.h
// ==============

#ifndef _INCLUDE_COMMON_COLOR_H_
#define _INCLUDE_COMMON_COLOR_H_

#include "common/common.h"

#include "vmath/vmath_common.h"
#include "vmath/vmath_vec4.h"

class Pixel32
{
public:
	typedef Pixel32 T;
	typedef uint8 ComponentType;
	typedef uint16 ComponentType_x2;
	typedef uint32 InternalType;
	enum { MAX_COMPONENT_VALUE = 255 };

	inline Pixel32() {}
	inline Pixel32(InternalType bgra_): bgra(bgra_) {}
	inline Pixel32(int r_,int g_,int b_,int a_=MAX_COMPONENT_VALUE): b((ComponentType)b_),g((ComponentType)g_),r((ComponentType)r_),a((ComponentType)a_) {}
	inline bool operator ==(const T& rhs) const { return bgra == rhs.bgra; }
	inline bool operator !=(const T& rhs) const { return bgra != rhs.bgra; }
	union {
		struct { ComponentType b,g,r,a; };
		struct { InternalType bgra; };
	};

	inline Pixel32(Vec3V_arg v) { Permute4<2,1,0,0>(v).StoreFixed8V(reinterpret_cast<uint8*>(this)); a = MAX_COMPONENT_VALUE; }
	inline Pixel32(Vec4V_arg v) { Permute4<2,1,0,3>(v).StoreFixed8V(reinterpret_cast<uint8*>(this)); }
	inline operator Vec3V() const { return Permute3<2,1,0>(Vec4V::LoadFixed8V(reinterpret_cast<const uint8*>(this))); }
	inline operator Vec4V() const { return Permute4<2,1,0,3>(Vec4V::LoadFixed8V(reinterpret_cast<const uint8*>(this))); }

	inline InternalType GetRGBA() const { return T(b,g,r,a).bgra; }

	static ComponentType Scale(ComponentType i, float t) { return (ComponentType)Clamp((int)(0.5f + (float)i*t), 0, MAX_COMPONENT_VALUE); }
	static ComponentType ScaleUINT(ComponentType i, ComponentType j) { return (ComponentType)Clamp((int)(0.5f + (float)((ComponentType_x2)i*(ComponentType_x2)j))/(float)MAX_COMPONENT_VALUE, 0, MAX_COMPONENT_VALUE); }
	static ComponentType BlendToUINT(ComponentType i, ComponentType j, float t) { return (ComponentType)Clamp((int)(0.5f + (float)i + (float)((int)j - (int)i)*t), 0, MAX_COMPONENT_VALUE); }

	inline const T ScaleColor(float t) const { return T(Scale(r,t),Scale(g,t),Scale(b,t),a); }
	inline const T ScaleAlpha(float t) const { return T(r,g,b,Scale(a,t)); }
	inline const T ScaleRGB(const T& rgb) const { return T(ScaleUINT(r,rgb.r),ScaleUINT(g,rgb.g),ScaleUINT(b,rgb.b),a); }
	inline const T ScaleRGBA(const T& rgba) const { return T(ScaleUINT(r,rgba.r),ScaleUINT(g,rgba.g),ScaleUINT(b,rgba.b),ScaleUINT(a,rgba.a)); }
	inline const T BlendToRGB(const T& rgb, float t) const { return T(BlendToUINT(r,rgb.r,t),BlendToUINT(g,rgb.g,t),BlendToUINT(b,rgb.b,t),a); }
	inline const T BlendToRGBA(const T& rgba, float t) const { return T(BlendToUINT(r,rgba.r,t),BlendToUINT(g,rgba.g,t),BlendToUINT(b,rgba.b,t),BlendToUINT(a,rgba.a,t)); }
};

class Pixel64
{
public:
	typedef Pixel64 T;
	typedef uint16 ComponentType;
	typedef uint32 ComponentType_x2;
	typedef uint64 InternalType;
	enum { MAX_COMPONENT_VALUE = 65535 };
	
	inline Pixel64() {}
	inline Pixel64(InternalType bgra_): bgra(bgra_) {}
	inline Pixel64(int r_,int g_,int b_,int a_=MAX_COMPONENT_VALUE): b((ComponentType)b_),g((ComponentType)g_),r((ComponentType)r_),a((ComponentType)a_) {}
	inline bool operator ==(const T& rhs) const { return bgra == rhs.bgra; }
	inline bool operator !=(const T& rhs) const { return bgra != rhs.bgra; }
	union {
		struct { ComponentType b,g,r,a; };
		struct { InternalType bgra; };
	};

	inline Pixel64(Vec3V_arg v) { Permute4<2,1,0,0>(v).StoreFixed16V(reinterpret_cast<uint16*>(this)); a = MAX_COMPONENT_VALUE; }
	inline Pixel64(Vec4V_arg v) { Permute4<2,1,0,3>(v).StoreFixed16V(reinterpret_cast<uint16*>(this)); }
	inline operator Vec3V() const { return Permute3<2,1,0>(Vec4V::LoadFixed16V(reinterpret_cast<const uint16*>(this))); }
	inline operator Vec4V() const { return Permute4<2,1,0,3>(Vec4V::LoadFixed16V(reinterpret_cast<const uint16*>(this))); }

	inline InternalType GetRGBA() const { return T(b,g,r,a).bgra; }

	static ComponentType Scale(ComponentType i, float t) { return (ComponentType)Clamp((int)(0.5f + (float)i*t), 0, MAX_COMPONENT_VALUE); }
	static ComponentType ScaleUINT(ComponentType i, ComponentType j) { return (ComponentType)Clamp((int)(0.5f + (float)((ComponentType_x2)i*(ComponentType_x2)j))/(float)MAX_COMPONENT_VALUE, 0, MAX_COMPONENT_VALUE); }
	static ComponentType BlendToUINT(ComponentType i, ComponentType j, float t) { return (ComponentType)Clamp((int)(0.5f + (float)i + (float)((int)j - (int)i)*t), 0, MAX_COMPONENT_VALUE); }

	inline const T ScaleColor(float t) const { return T(Scale(r,t),Scale(g,t),Scale(b,t),a); }
	inline const T ScaleAlpha(float t) const { return T(r,g,b,Scale(a,t)); }
	inline const T ScaleRGB(const T& rgb) const { return T(ScaleUINT(r,rgb.r),ScaleUINT(g,rgb.g),ScaleUINT(b,rgb.b),a); }
	inline const T ScaleRGBA(const T& rgba) const { return T(ScaleUINT(r,rgba.r),ScaleUINT(g,rgba.g),ScaleUINT(b,rgba.b),ScaleUINT(a,rgba.a)); }
	inline const T BlendToRGB(const T& rgb, float t) const { return T(BlendToUINT(r,rgb.r,t),BlendToUINT(g,rgb.g,t),BlendToUINT(b,rgb.b,t),a); }
	inline const T BlendToRGBA(const T& rgba, float t) const { return T(BlendToUINT(r,rgba.r,t),BlendToUINT(g,rgba.g,t),BlendToUINT(b,rgba.b,t),BlendToUINT(a,rgba.a,t)); }
};

// forward declare pixel types
template <typename ElementType> class Vec3_T;
typedef Vec3_T<float> Vec3f;

#define PIXELTYPE_uint8   uint8
#define PIXELTYPE_uint16  uint16
#define PIXELTYPE_float   float
#define PIXELTYPE_Vec3f   Vec3f
#define PIXELTYPE_Vec3V   Vec3V
#define PIXELTYPE_Vec4V   Vec4V
#define PIXELTYPE_Pixel32 Pixel32
#define PIXELTYPE_Pixel64 Pixel64

#endif // _INCLUDE_COMMON_COLOR_H_
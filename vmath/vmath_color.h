// ==========================
// common/vmath/vmath_color.h
// ==========================

#ifndef _INCLUDE_VMATH_COLOR_H_
#define _INCLUDE_VMATH_COLOR_H_

#include "vmath_common.h"
#include "vmath_floatvec.h"
#include "vmath_vec4.h"
#if 1//defined(_OFFLINETOOL)
#include "GraphicsTools/util/color.h"
#else
class Pixel32
{
public:
	inline Pixel32() {}
	inline Pixel32(uint32 bgra_): bgra(bgra_) {}
	inline Pixel32(int r_,int g_,int b_,int a_=255): b((uint8)b_),g((uint8)g_),r((uint8)r_),a((uint8)a_) {}
	inline Pixel32(Vec3V_arg v) { Permute4<2,1,0,0>(v).StoreFixed8V(reinterpret_cast<uint8*>(this)); a = 255; }
	inline Pixel32(Vec4V_arg v) { Permute4<2,1,0,3>(v).StoreFixed8V(reinterpret_cast<uint8*>(this)); }
	inline operator Vec3V() const { return Permute3<2,1,0>(Vec4V::LoadFixed8V(reinterpret_cast<const uint8*>(this))); }
	inline operator Vec4V() const { return Permute4<2,1,0,3>(Vec4V::LoadFixed8V(reinterpret_cast<const uint8*>(this))); }
	inline bool operator ==(const Pixel32& rhs) const { return bgra == rhs.bgra; }
	inline bool operator !=(const Pixel32& rhs) const { return bgra != rhs.bgra; }
	union {
		struct { uint8 b,g,r,a; };
		struct { uint32 bgra; };
	};
};

class Pixel64
{
public:
	inline Pixel64() {}
	inline Pixel64(uint64 bgra_): bgra(bgra_) {}
	inline Pixel64(int r_,int g_,int b_,int a_=65535): b((uint16)b_),g((uint16)g_),r((uint16)r_),a((uint16)a_) {}
	inline Pixel64(Vec3V_arg v) { Permute4<2,1,0,0>(v).StoreFixed16V(reinterpret_cast<uint16*>(this)); a = 65535; }
	inline Pixel64(Vec4V_arg v) { Permute4<2,1,0,3>(v).StoreFixed16V(reinterpret_cast<uint16*>(this)); }
	inline operator Vec3V() const { return Permute3<2,1,0>(Vec4V::LoadFixed16V(reinterpret_cast<const uint16*>(this))); }
	inline operator Vec4V() const { return Permute4<2,1,0,3>(Vec4V::LoadFixed16V(reinterpret_cast<const uint16*>(this))); }
	inline bool operator ==(const Pixel64& rhs) const { return bgra == rhs.bgra; }
	inline bool operator !=(const Pixel64& rhs) const { return bgra != rhs.bgra; }
	union {
		struct { uint16 b,g,r,a; };
		struct { uint64 bgra; };
	};
};
#define PIXELTYPE_uint8 uint8
#define PIXELTYPE_uint16 uint16
#define PIXELTYPE_float float
#define PIXELTYPE_Vec3f Vec3f
#define PIXELTYPE_Vec3V Vec3V
#define PIXELTYPE_Vec4V Vec4V
#define PIXELTYPE_Pixel32 Pixel32
#define PIXELTYPE_Pixel64 Pixel64
#endif

enum PixelType
{
	PixelType_unknown,
	PixelType_uint8,
	PixelType_uint16,
	PixelType_float,
	PixelType_Vec3f,
	PixelType_Vec3V,
	PixelType_Vec4V,
	PixelType_Pixel32,
	PixelType_Pixel64,
};

template <typename T_> class GetPixelType { public: enum { N = 0, T = PixelType_unknown }; static const char* Str() { return "unknown"; } };
template <> class GetPixelType<PIXELTYPE_uint8  >   { public: enum { N = 1, T = PixelType_uint8   }; static const char* Str() { return "uint8"  ; } };
template <> class GetPixelType<PIXELTYPE_uint16 >   { public: enum { N = 1, T = PixelType_uint16  }; static const char* Str() { return "uint16" ; } };
template <> class GetPixelType<PIXELTYPE_float  >   { public: enum { N = 1, T = PixelType_float   }; static const char* Str() { return "float"  ; } };
template <> class GetPixelType<PIXELTYPE_Vec3f  >   { public: enum { N = 3, T = PixelType_Vec3f   }; static const char* Str() { return "Vec3f"  ; } };
template <> class GetPixelType<PIXELTYPE_Vec3V  >   { public: enum { N = 3, T = PixelType_Vec3V   }; static const char* Str() { return "Vec3V"  ; } };
template <> class GetPixelType<PIXELTYPE_Vec4V  >   { public: enum { N = 4, T = PixelType_Vec4V   }; static const char* Str() { return "Vec4V"  ; } };
template <> class GetPixelType<PIXELTYPE_Pixel32>   { public: enum { N = 4, T = PixelType_Pixel32 }; static const char* Str() { return "Pixel32"; } };
template <> class GetPixelType<PIXELTYPE_Pixel64>   { public: enum { N = 4, T = PixelType_Pixel64 }; static const char* Str() { return "Pixel64"; } };

template <typename DstType,typename SrcType> VMATH_INLINE const DstType ConvertPixel(const SrcType& src);

// these must be declared "inline", and not VMATH_INLINE, due to compiler weirdness ..
template <> inline const PIXELTYPE_uint8   ConvertPixel<PIXELTYPE_uint8  ,uint8>(const uint8& p) { return p; }
template <> inline const PIXELTYPE_uint16  ConvertPixel<PIXELTYPE_uint16 ,uint8>(const uint8& p) { const uint16 q = p; return q|(q<<8); }
template <> inline const PIXELTYPE_float   ConvertPixel<PIXELTYPE_float  ,uint8>(const uint8& p) { return (float)p/255.0f; }
template <> inline const PIXELTYPE_Vec3f   ConvertPixel<PIXELTYPE_Vec3f  ,uint8>(const uint8& p) { const float q = ConvertPixel<float,uint8>(p); return Vec3f(q); }
template <> inline const PIXELTYPE_Vec3V   ConvertPixel<PIXELTYPE_Vec3V  ,uint8>(const uint8& p) { const float q = ConvertPixel<float,uint8>(p); return Vec3V(q); }
template <> inline const PIXELTYPE_Vec4V   ConvertPixel<PIXELTYPE_Vec4V  ,uint8>(const uint8& p) { const float q = ConvertPixel<float,uint8>(p); return Vec4V(q,q,q,1.0f); }
template <> inline const PIXELTYPE_Pixel32 ConvertPixel<PIXELTYPE_Pixel32,uint8>(const uint8& p) { return Pixel32(p,p,p); }
template <> inline const PIXELTYPE_Pixel64 ConvertPixel<PIXELTYPE_Pixel64,uint8>(const uint8& p) { const uint16 q = ConvertPixel<uint16,uint8>(p); return Pixel64(q,q,q); }

template <> inline const PIXELTYPE_uint8   ConvertPixel<PIXELTYPE_uint8  ,uint16>(const uint16& p) { return (uint8)(p>>8); }
template <> inline const PIXELTYPE_uint16  ConvertPixel<PIXELTYPE_uint16 ,uint16>(const uint16& p) { return p; }
template <> inline const PIXELTYPE_float   ConvertPixel<PIXELTYPE_float  ,uint16>(const uint16& p) { return (float)p/65535.0f; }
template <> inline const PIXELTYPE_Vec3f   ConvertPixel<PIXELTYPE_Vec3f  ,uint16>(const uint16& p) { const float q = ConvertPixel<float,uint16>(p); return Vec3f(q); }
template <> inline const PIXELTYPE_Vec3V   ConvertPixel<PIXELTYPE_Vec3V  ,uint16>(const uint16& p) { const float q = ConvertPixel<float,uint16>(p); return Vec3V(q); }
template <> inline const PIXELTYPE_Vec4V   ConvertPixel<PIXELTYPE_Vec4V  ,uint16>(const uint16& p) { const float q = ConvertPixel<float,uint16>(p); return Vec4V(q,q,q,1.0f); }
template <> inline const PIXELTYPE_Pixel32 ConvertPixel<PIXELTYPE_Pixel32,uint16>(const uint16& p) { const uint8 q = ConvertPixel<uint8,uint16>(p); return Pixel32(q,q,q); }
template <> inline const PIXELTYPE_Pixel64 ConvertPixel<PIXELTYPE_Pixel64,uint16>(const uint16& p) { return Pixel64(p,p,p); }

template <> inline const PIXELTYPE_uint8   ConvertPixel<PIXELTYPE_uint8  ,float>(const float& p) { return (uint8)Clamp(0.5f + 255.0f*p,0.0f,255.0f); }
template <> inline const PIXELTYPE_uint16  ConvertPixel<PIXELTYPE_uint16 ,float>(const float& p) { return (uint16)Clamp(0.5f + 65535.0f*p,0.0f,65535.0f); }
template <> inline const PIXELTYPE_float   ConvertPixel<PIXELTYPE_float  ,float>(const float& p) { return p; }
template <> inline const PIXELTYPE_Vec3f   ConvertPixel<PIXELTYPE_Vec3f  ,float>(const float& p) { return Vec3f(p); }
template <> inline const PIXELTYPE_Vec3V   ConvertPixel<PIXELTYPE_Vec3V  ,float>(const float& p) { return Vec3V(p); }
template <> inline const PIXELTYPE_Vec4V   ConvertPixel<PIXELTYPE_Vec4V  ,float>(const float& p) { return Vec4V(p,p,p,1.0f); }
template <> inline const PIXELTYPE_Pixel32 ConvertPixel<PIXELTYPE_Pixel32,float>(const float& p) { const uint8 q = ConvertPixel<uint8,float>(p); return Pixel32(q,q,q); }
template <> inline const PIXELTYPE_Pixel64 ConvertPixel<PIXELTYPE_Pixel64,float>(const float& p) { const uint16 q = ConvertPixel<uint16,float>(p); return Pixel64(q,q,q); }

template <> inline const PIXELTYPE_uint8   ConvertPixel<PIXELTYPE_uint8  ,Vec3f>(const Vec3f& p) { return ConvertPixel<uint8,float>(p.x()); }
template <> inline const PIXELTYPE_uint16  ConvertPixel<PIXELTYPE_uint16 ,Vec3f>(const Vec3f& p) { return ConvertPixel<uint16,float>(p.x()); }
template <> inline const PIXELTYPE_float   ConvertPixel<PIXELTYPE_float  ,Vec3f>(const Vec3f& p) { return p.x(); }
template <> inline const PIXELTYPE_Vec3f   ConvertPixel<PIXELTYPE_Vec3f  ,Vec3f>(const Vec3f& p) { return p; }
template <> inline const PIXELTYPE_Vec3V   ConvertPixel<PIXELTYPE_Vec3V  ,Vec3f>(const Vec3f& p) { return Vec3V(VEC3V_ARGS(p)); }
template <> inline const PIXELTYPE_Vec4V   ConvertPixel<PIXELTYPE_Vec4V  ,Vec3f>(const Vec3f& p) { return Vec4V(VEC3V_ARGS(p),1.0f); }
template <> inline const PIXELTYPE_Pixel32 ConvertPixel<PIXELTYPE_Pixel32,Vec3f>(const Vec3f& p) { return Pixel32(ConvertPixel<uint8,float>(p.x()),ConvertPixel<uint8,float>(p.y()),ConvertPixel<uint8,float>(p.z())); }
template <> inline const PIXELTYPE_Pixel64 ConvertPixel<PIXELTYPE_Pixel64,Vec3f>(const Vec3f& p) { return Pixel64(ConvertPixel<uint16,float>(p.x()),ConvertPixel<uint16,float>(p.y()),ConvertPixel<uint16,float>(p.z())); }

template <> inline const PIXELTYPE_uint8   ConvertPixel<PIXELTYPE_uint8  ,Vec4V>(const Vec4V& p) { return ConvertPixel<uint8,float>(p.xf()); }
template <> inline const PIXELTYPE_uint16  ConvertPixel<PIXELTYPE_uint16 ,Vec4V>(const Vec4V& p) { return ConvertPixel<uint16,float>(p.xf()); }
template <> inline const PIXELTYPE_float   ConvertPixel<PIXELTYPE_float  ,Vec4V>(const Vec4V& p) { return p.xf(); }
template <> inline const PIXELTYPE_Vec3f   ConvertPixel<PIXELTYPE_Vec3f  ,Vec4V>(const Vec4V& p) { return Vec3f(VEC3V_ARGS(p)); }
template <> inline const PIXELTYPE_Vec3V   ConvertPixel<PIXELTYPE_Vec3V  ,Vec4V>(const Vec4V& p) { return Vec3V(VEC3V_ARGS(p)); }
template <> inline const PIXELTYPE_Vec4V   ConvertPixel<PIXELTYPE_Vec4V  ,Vec4V>(const Vec4V& p) { return p; }
template <> inline const PIXELTYPE_Pixel32 ConvertPixel<PIXELTYPE_Pixel32,Vec4V>(const Vec4V& p) { return Pixel32(ConvertPixel<uint8,float>(p.xf()),ConvertPixel<uint8,float>(p.yf()),ConvertPixel<uint8,float>(p.zf()),ConvertPixel<uint8,float>(p.wf())); }
template <> inline const PIXELTYPE_Pixel64 ConvertPixel<PIXELTYPE_Pixel64,Vec4V>(const Vec4V& p) { return Pixel64(ConvertPixel<uint16,float>(p.xf()),ConvertPixel<uint16,float>(p.yf()),ConvertPixel<uint16,float>(p.zf()),ConvertPixel<uint16,float>(p.wf())); }

template <> inline const PIXELTYPE_uint8   ConvertPixel<PIXELTYPE_uint8  ,Pixel32>(const Pixel32& p) { return p.r; }
template <> inline const PIXELTYPE_uint16  ConvertPixel<PIXELTYPE_uint16 ,Pixel32>(const Pixel32& p) { return ConvertPixel<uint16,uint8>(p.r); }
template <> inline const PIXELTYPE_float   ConvertPixel<PIXELTYPE_float  ,Pixel32>(const Pixel32& p) { return ConvertPixel<float,uint8>(p.r); }
template <> inline const PIXELTYPE_Vec3f   ConvertPixel<PIXELTYPE_Vec3f  ,Pixel32>(const Pixel32& p) { return Vec3f(ConvertPixel<float,uint8>(p.r),ConvertPixel<float,uint8>(p.g),ConvertPixel<float,uint8>(p.b)); }
template <> inline const PIXELTYPE_Vec3V   ConvertPixel<PIXELTYPE_Vec3V  ,Pixel32>(const Pixel32& p) { return Vec3V(ConvertPixel<float,uint8>(p.r),ConvertPixel<float,uint8>(p.g),ConvertPixel<float,uint8>(p.b)); }
template <> inline const PIXELTYPE_Vec4V   ConvertPixel<PIXELTYPE_Vec4V  ,Pixel32>(const Pixel32& p) { return Vec4V(ConvertPixel<float,uint8>(p.r),ConvertPixel<float,uint8>(p.g),ConvertPixel<float,uint8>(p.b),ConvertPixel<float,uint8>(p.a)); }
template <> inline const PIXELTYPE_Pixel32 ConvertPixel<PIXELTYPE_Pixel32,Pixel32>(const Pixel32& p) { return p; }
template <> inline const PIXELTYPE_Pixel64 ConvertPixel<PIXELTYPE_Pixel64,Pixel32>(const Pixel32& p) { return Pixel64(ConvertPixel<uint16,uint8>(p.r),ConvertPixel<uint16,uint8>(p.g),ConvertPixel<uint16,uint8>(p.b),ConvertPixel<uint16,uint8>(p.a)); }

template <> inline const PIXELTYPE_uint8   ConvertPixel<PIXELTYPE_uint8  ,Pixel64>(const Pixel64& p) { return ConvertPixel<uint8,uint16>(p.r); }
template <> inline const PIXELTYPE_uint16  ConvertPixel<PIXELTYPE_uint16 ,Pixel64>(const Pixel64& p) { return p.r; }
template <> inline const PIXELTYPE_float   ConvertPixel<PIXELTYPE_float  ,Pixel64>(const Pixel64& p) { return ConvertPixel<float,uint16>(p.r); }
template <> inline const PIXELTYPE_Vec3f   ConvertPixel<PIXELTYPE_Vec3f  ,Pixel64>(const Pixel64& p) { return Vec3f(ConvertPixel<float,uint16>(p.r),ConvertPixel<float,uint16>(p.g),ConvertPixel<float,uint16>(p.b)); }
template <> inline const PIXELTYPE_Vec3V   ConvertPixel<PIXELTYPE_Vec3V  ,Pixel64>(const Pixel64& p) { return Vec3V(ConvertPixel<float,uint16>(p.r),ConvertPixel<float,uint16>(p.g),ConvertPixel<float,uint16>(p.b)); }
template <> inline const PIXELTYPE_Vec4V   ConvertPixel<PIXELTYPE_Vec4V  ,Pixel64>(const Pixel64& p) { return Vec4V(ConvertPixel<float,uint16>(p.r),ConvertPixel<float,uint16>(p.g),ConvertPixel<float,uint16>(p.b),ConvertPixel<float,uint16>(p.a)); }
template <> inline const PIXELTYPE_Pixel32 ConvertPixel<PIXELTYPE_Pixel32,Pixel64>(const Pixel64& p) { return Pixel32(ConvertPixel<uint8,uint16>(p.r),ConvertPixel<uint8,uint16>(p.g),ConvertPixel<uint8,uint16>(p.b),ConvertPixel<uint8,uint16>(p.a)); }
template <> inline const PIXELTYPE_Pixel64 ConvertPixel<PIXELTYPE_Pixel64,Pixel64>(const Pixel64& p) { return p; }

//template <> inline Vec3V_out ConvertPixel<Vec3V,Pixel32>(const Pixel32& p) { return p; }
//template <> inline const Pixel32 ConvertPixel<Pixel32,Vec3V>(const Vec3V& p) { return Pixel32(p); }

VMATH_INLINE float LinearToSRGB_float(float value) { if (value <= 0.0031308f) return value*12.92f; else return 1.055f*powf(value,1.0f/2.4f) - 0.055f; }
VMATH_INLINE float SRGBtoLinear_float(float value) { if (value <= 0.04045f) return value/12.92f; else return powf((value + 0.055f)/1.055f,2.4f); }

template <typename T> VMATH_INLINE const T LinearToSRGB(const T& value) { return ConvertPixel<T,float>(LinearToSRGB_float(ConvertPixel<float,T>(value))); }
template <typename T> VMATH_INLINE const T SRGBtoLinear(const T& value) { return ConvertPixel<T,float>(SRGBtoLinear_float(ConvertPixel<float,T>(value))); }

// these must be declared "inline", and not VMATH_INLINE, due to compiler weirdness ..
template <> inline Vec3f_out     LinearToSRGB<Vec3f  >(const Vec3f  & value) { return Vec3f  (LinearToSRGB(value.xf()),LinearToSRGB(value.yf()),LinearToSRGB(value.zf())); }
template <> inline Vec3V_out     LinearToSRGB<Vec3V  >(const Vec3V  & value) { return Vec3V  (LinearToSRGB(value.xf()),LinearToSRGB(value.yf()),LinearToSRGB(value.zf())); }
template <> inline Vec4f_out     LinearToSRGB<Vec4f  >(const Vec4f  & value) { return Vec4f  (LinearToSRGB(value.xf()),LinearToSRGB(value.yf()),LinearToSRGB(value.zf()),value.wf()); }
template <> inline Vec4V_out     LinearToSRGB<Vec4V  >(const Vec4V  & value) { return Vec4V  (LinearToSRGB(value.xf()),LinearToSRGB(value.yf()),LinearToSRGB(value.zf()),value.wf()); }
template <> inline const Pixel32 LinearToSRGB<Pixel32>(const Pixel32& value) { return Pixel32(LinearToSRGB(value.r   ),LinearToSRGB(value.g   ),LinearToSRGB(value.b   ),value.a); }
template <> inline const Pixel64 LinearToSRGB<Pixel64>(const Pixel64& value) { return Pixel64(LinearToSRGB(value.r   ),LinearToSRGB(value.g   ),LinearToSRGB(value.b   ),value.a); }

template <> inline Vec3f_out     SRGBtoLinear<Vec3f  >(const Vec3f  & value) { return Vec3f  (SRGBtoLinear(value.xf()),SRGBtoLinear(value.yf()),SRGBtoLinear(value.zf())); }
template <> inline Vec3V_out     SRGBtoLinear<Vec3V  >(const Vec3V  & value) { return Vec3V  (SRGBtoLinear(value.xf()),SRGBtoLinear(value.yf()),SRGBtoLinear(value.zf())); }
template <> inline Vec4f_out     SRGBtoLinear<Vec4f  >(const Vec4f  & value) { return Vec4f  (SRGBtoLinear(value.xf()),SRGBtoLinear(value.yf()),SRGBtoLinear(value.zf()),value.wf()); }
template <> inline Vec4V_out     SRGBtoLinear<Vec4V  >(const Vec4V  & value) { return Vec4V  (SRGBtoLinear(value.xf()),SRGBtoLinear(value.yf()),SRGBtoLinear(value.zf()),value.wf()); }
template <> inline const Pixel32 SRGBtoLinear<Pixel32>(const Pixel32& value) { return Pixel32(SRGBtoLinear(value.r   ),SRGBtoLinear(value.g   ),SRGBtoLinear(value.b   ),value.a); }
template <> inline const Pixel64 SRGBtoLinear<Pixel64>(const Pixel64& value) { return Pixel64(SRGBtoLinear(value.r   ),SRGBtoLinear(value.g   ),SRGBtoLinear(value.b   ),value.a); }

#endif // _INCLUDE_VMATH_COLOR_H_

// ==================
// common/imageutil.h
// ==================

#ifndef _INCLUDE_COMMON_IMAGEUTIL_H_
#define _INCLUDE_COMMON_IMAGEUTIL_H_

#include "common/common.h"

#include "color.h"
#include "dds.h" // for DDS_DXGI_FORMAT

#if defined(_FREEIMAGE)
#define FREEIMAGE_LIB
#include "GraphicsTools/external/FreeImage/FreeImage.h"
// NOTE -- i compiled FreeImage for vs2017 by copying the 2013 projects and renaming to 2017
// i also had to remove a #define for snprintf in tif_config.h
// then i built the FreeImageLib project for x64, both Debug and Release
#if defined(_DEBUG)
#define FREEIMAGE_LIB_STR "Libd.lib"
#else
#define FREEIMAGE_LIB_STR "Lib.lib"
#endif
#pragma comment(lib, "GraphicsTools/external/FreeImage/x64/FreeImage" VISUAL_STUDIO_VERSION_STR FREEIMAGE_LIB_STR)
#define DEFAULT_IMAGE_EXT ".png"
#else
#define DEFAULT_IMAGE_EXT ".dds"
#endif
#undef LoadImage // windows.h defines this, boo

bool GetImageDimensions(const char* path, int& w, int& h, DDS_DXGI_FORMAT* out_format = NULL);

// at one point i had a custom build of FreeImage that allowed reading TIFF layers ..
#define TIFF_SAMPLE_INDEX_ONLY(...) //__VA_ARGS__

template <typename DstType> DstType* LoadImage_T(const char* path, int& w, int& h TIFF_SAMPLE_INDEX_ONLY(, int sampleIndex = -1));

PIXELTYPE_uint8  * LoadImage_uint8  (const char* path, int& w, int& h TIFF_SAMPLE_INDEX_ONLY(, int sampleIndex = -1));
PIXELTYPE_uint16 * LoadImage_uint16 (const char* path, int& w, int& h TIFF_SAMPLE_INDEX_ONLY(, int sampleIndex = -1));
PIXELTYPE_float  * LoadImage_float  (const char* path, int& w, int& h TIFF_SAMPLE_INDEX_ONLY(, int sampleIndex = -1));
PIXELTYPE_Vec3f  * LoadImage_Vec3f  (const char* path, int& w, int& h TIFF_SAMPLE_INDEX_ONLY(, int sampleIndex = -1));
PIXELTYPE_Vec4V  * LoadImage_Vec4V  (const char* path, int& w, int& h TIFF_SAMPLE_INDEX_ONLY(, int sampleIndex = -1));
PIXELTYPE_Pixel32* LoadImage_Pixel32(const char* path, int& w, int& h TIFF_SAMPLE_INDEX_ONLY(, int sampleIndex = -1));
PIXELTYPE_Pixel64* LoadImage_Pixel64(const char* path, int& w, int& h TIFF_SAMPLE_INDEX_ONLY(, int sampleIndex = -1));

PIXELTYPE_uint8  * LoadImageDDSSpan_uint8  (const char* path, int w, int y0, int h);
PIXELTYPE_uint16 * LoadImageDDSSpan_uint16 (const char* path, int w, int y0, int h);
PIXELTYPE_float  * LoadImageDDSSpan_float  (const char* path, int w, int y0, int h);
PIXELTYPE_Vec3f  * LoadImageDDSSpan_Vec3f  (const char* path, int w, int y0, int h);
PIXELTYPE_Vec4V  * LoadImageDDSSpan_Vec4V  (const char* path, int w, int y0, int h);
PIXELTYPE_Pixel32* LoadImageDDSSpan_Pixel32(const char* path, int w, int y0, int h);
PIXELTYPE_Pixel64* LoadImageDDSSpan_Pixel64(const char* path, int w, int y0, int h);

bool SaveImage(const char* path, const PIXELTYPE_uint8  * image, int w, int h, bool bAutoOpen = false, bool bNative = false);
bool SaveImage(const char* path, const PIXELTYPE_uint16 * image, int w, int h, bool bAutoOpen = false, bool bNative = false);
bool SaveImage(const char* path, const PIXELTYPE_float  * image, int w, int h, bool bAutoOpen = false, bool bNative = false);
bool SaveImage(const char* path, const PIXELTYPE_Vec3f  * image, int w, int h, bool bAutoOpen = false, bool bNative = false);
bool SaveImage(const char* path, const PIXELTYPE_Vec4V  * image, int w, int h, bool bAutoOpen = false, bool bNative = false);
bool SaveImage(const char* path, const PIXELTYPE_Pixel32* image, int w, int h, bool bAutoOpen = false, bool bNative = false);
bool SaveImage(const char* path, const PIXELTYPE_Pixel64* image, int w, int h, bool bAutoOpen = false, bool bNative = false);

template <typename T> static bool SaveImageAtlas3D(const char* path, const T* image, int w, int h, int d, const T& fill = T(0))
{
	fprintf(stdout, "saving image atlas %s .. ", path);
	fflush(stdout);
	const int cols = (int)Ceiling(Sqrt((float)d));
	const int rows = (d + cols - 1)/cols;
	const int atlas_w = cols*w;
	const int atlas_h = rows*h;
	T* atlas = new T[atlas_w*atlas_h];
	for (int row = 0; row < rows; row++) {
		for (int col = 0; col < cols; col++) {
			const int k = col + row*cols;
			for (int j = 0; j < h; j++) {
				for (int i = 0; i < w; i++) {
					T& dst = atlas[(i + col*w) + (j + row*h)*atlas_w];
					if (k < d)
						dst = image[i + (j + k*h)*w];
					else
						dst = fill;
				}
			}
		}
	}
	const bool result = SaveImage(path, atlas, atlas_w, atlas_h);
	delete[] atlas;
	if (result)
		fprintf(stdout, "ok.\n");
	else
		fprintf(stderr, "failed!\n");
	return result;
}

//template <typename DstType,typename SrcType> inline DstType* ConvertImage(const SrcType* image, int w, int h)
//{
//	DstType* dst = new DstType[w*h];
//	for (int i = 0; i < w*h; i++)
//		dst[i] = ConvertPixel<DstType,SrcType>(image[i]);
//	return dst;
//}

Pixel32 GetUniqueColor(uint32 index);

float* ImageGraph(float* dst, const float* src, int w, int h = 0);
float* ImageGraph(float* dst, const double* src, int w, int h = 0);

float* ImageGraph(float* dst, int w, int h, const float* graph, float rangeMin, float rangeMax, float thickness);
void ImageGraphOverlay(Vec4V* image, int w, int h, const float* graph, float rangeMin, float rangeMax, float thickness, Vec3V_arg color, float opacity);
Vec4V* ImageGraphSet(int w, int h, const std::vector<const std::vector<float>*>& graphs, float thickness);

uint8* ImageHistogram(const float* data, size_t count, uint32 w, uint32 h, uint32 numBuckets = 32, float* minValue_inout = nullptr, float* maxValue_inout = nullptr, float scale = 0.9f);
uint8* ImageHistogram(const double* data, size_t count, uint32 w, uint32 h, uint32 numBuckets = 32, double* minValue_inout = nullptr, double* maxValue_inout = nullptr, float scale = 0.9f);

template <typename T> inline T Sample1D(float coord, uint32 w, const T* data, bool filter, bool sampleFromCenters = false)
{
	const float res = (float)w;
	const float res_minus_1 = res - 1.0f;
	const float p = sampleFromCenters ? coord*res_minus_1 : (coord*res - 0.5f); // [0..w] within bounds
	if (filter) {
		const float p0 = Floor(p);
		const float p1 = p0 + 1.0f;
		const float p0_int = Clamp(p0, 0.0f, res_minus_1);
		const float p1_int = Clamp(p1, 0.0f, res_minus_1);
		const float pfrac = p - p0;
		const T v0 = data[(int)p0_int];
		const T v1 = data[(int)p1_int];
		const T vx = v0 + (v1 - v0)*pfrac;
		return vx;
	} else {
		const float p0 = Round(p);
		const float p0_int = Clamp(p0, 0.0f, res_minus_1);
		return data[(int)p0_int];
	}
}

template <typename T> inline T Sample2D(Vec2V_arg coord, uint32 w, uint32 h, const T* data, bool filter, bool sampleFromCenters = false)
{
	const Vec2V res((float)w, (float)h);
	const Vec2V res_minus_1 = res - ScalarV(V_ONE);
	const Vec2V p = sampleFromCenters ? coord*res_minus_1 : (coord*res - ScalarV(0.5f)); // [0..wh] within bounds
	if (filter) {
		const Vec2V p0 = Floor(p);
		const Vec2V p1 = p0 + ScalarV(V_ONE);
		const Vec2V p0_int = Clamp(p0, Vec2V(V_ZERO), res_minus_1);
		const Vec2V p1_int = Clamp(p1, Vec2V(V_ZERO), res_minus_1);
		const Vec2V pfrac = p - p0;
		const int i0 = (int)p0_int.xf();
		const int j0 = (int)p0_int.yf();
		const int i1 = (int)p1_int.xf();
		const int j1 = (int)p1_int.yf();
		const T v00 = data[i0 + j0*w];
		const T v10 = data[i1 + j0*w];
		const T v01 = data[i0 + j1*w];
		const T v11 = data[i1 + j1*w];
		const T vx0 = v00 + (v10 - v00)*pfrac.xf();
		const T vx1 = v01 + (v11 - v01)*pfrac.xf();
		const T vxy = vx0 + (vx1 - vx0)*pfrac.yf();
		return vxy;
	} else {
		const Vec2V p0 = Round(p);
		const Vec2V p0_int = Clamp(p0, Vec2V(V_ZERO), res_minus_1);
		const int i0 = (int)p0_int.xf();
		const int j0 = (int)p0_int.yf();
		return data[i0 + j0*w];
	}
}

template <typename T> inline T Sample3D(Vec3V_arg coord, uint32 w, uint32 h, uint32 d, const T* data, bool filter, bool sampleFromCenters = false)
{
	const Vec3V res((float)w, (float)h, (float)d);
	const Vec3V res_minus_1 = res - ScalarV(V_ONE);
	const Vec3V p = sampleFromCenters ? coord*res_minus_1 : (coord*res - ScalarV(0.5f)); // [0..whd] within bounds
	if (filter) {
		const Vec3V p0 = Floor(p);
		const Vec3V p1 = p0 + ScalarV(V_ONE);
		const Vec3V p0_int = Clamp(p0, Vec3V(V_ZERO), res_minus_1);
		const Vec3V p1_int = Clamp(p1, Vec3V(V_ZERO), res_minus_1);
		const Vec3V pfrac = p - p0;
		const int i0 = (int)p0_int.xf();
		const int j0 = (int)p0_int.yf();
		const int k0 = (int)p0_int.zf();
		const int i1 = (int)p1_int.xf();
		const int j1 = (int)p1_int.yf();
		const int k1 = (int)p1_int.zf();
		const T v000 = data[i0 + (j0 + k0*h)*w];
		const T v100 = data[i1 + (j0 + k0*h)*w];
		const T v010 = data[i0 + (j1 + k0*h)*w];
		const T v110 = data[i1 + (j1 + k0*h)*w];
		const T v001 = data[i0 + (j0 + k1*h)*w];
		const T v101 = data[i1 + (j0 + k1*h)*w];
		const T v011 = data[i0 + (j1 + k1*h)*w];
		const T v111 = data[i1 + (j1 + k1*h)*w];
		const T vx00 = v000 + (v100 - v000)*pfrac.xf();
		const T vx10 = v010 + (v110 - v010)*pfrac.xf();
		const T vx01 = v001 + (v101 - v001)*pfrac.xf();
		const T vx11 = v011 + (v111 - v011)*pfrac.xf();
		const T vxy0 = vx00 + (vx10 - vx00)*pfrac.yf();
		const T vxy1 = vx01 + (vx11 - vx01)*pfrac.yf();
		const T vxyz = vxy0 + (vxy1 - vxy0)*pfrac.zf();
		return vxyz;
	} else {
		const Vec3V p0 = Round(p);
		const Vec3V p0_int = Clamp(p0, Vec3V(V_ZERO), res_minus_1);
		const int i0 = (int)p0_int.xf();
		const int j0 = (int)p0_int.yf();
		const int k0 = (int)p0_int.zf();
		return data[i0 + (j0 + k0*h)*w];
	}
}

#if PLATFORM_PC
void ImageHistogramTest();
#endif // PLATFORM_PC

#if defined(_DEBUG)
void ImageTest();
#endif // defined(_DEBUG)

#endif // _INCLUDE_COMMON_IMAGEUTIL_H_
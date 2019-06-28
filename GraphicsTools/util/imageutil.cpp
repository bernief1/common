// ====================
// common/imageutil.cpp
// ====================

#include "imageutil.h"

#include "vmath/vmath_color.h"
#include "vmath/vmath_vec4.h"

#if PLATFORM_PC
#include <random>
#endif

#if defined(_FREEIMAGE)

static void FreeImageInit()
{
	static bool once = true;

	if (once)
	{
		FreeImage_Initialise();
		once = false;
	}
}

static bool FreeImageIsFlippedVertically(FIBITMAP* dib, FREE_IMAGE_FORMAT fif)
{
	bool flipped = false;

	if (fif == FIF_TIFF)
	{
		FITAG* tag = NULL;
		FIMETADATA* md = FreeImage_FindFirstMetadata(FIMD_EXIF_MAIN, dib, &tag);

		if (md)
		{
			int n = 0; do
			{
				if (++n > 1000) break; // just in case

				if (strcmp(FreeImage_GetTagKey(tag), "Orientation") == 0)
				{
					enum
					{
						ORIENTATION_TOPLEFT  = 1,
						ORIENTATION_TOPRIGHT = 2,
						ORIENTATION_BOTRIGHT = 3,
						ORIENTATION_BOTLEFT  = 4,
						ORIENTATION_LEFTTOP  = 5,
						ORIENTATION_RIGHTTOP = 6,
						ORIENTATION_RIGHTBOT = 7,
						ORIENTATION_LEFTBOT  = 8, 
					};
					DEBUG_ASSERT(FreeImage_GetTagType(tag) == FIDT_SHORT);
					const int16 orientation = *(const int16*)FreeImage_GetTagValue(tag);
					DEBUG_ASSERT(orientation == ORIENTATION_TOPLEFT || orientation == ORIENTATION_BOTLEFT); // is any other orientation in use?

					if (orientation == ORIENTATION_BOTLEFT || orientation == ORIENTATION_BOTRIGHT)
					{
						flipped = true;
						break;
					}
				}
			} while (FreeImage_FindNextMetadata(md, &tag));
			FreeImage_FindCloseMetadata(md);
		}
	}

	return flipped;
}

#endif // defined(_FREEIMAGE)

bool GetImageDimensions(const char* path, int& w, int& h, DDS_DXGI_FORMAT* out_format)
{
	const char* ext = strrchr(path, '.');

	if (ext && stricmp(ext, ".dds") == 0)
	{
		FILE* ddsFile = fopen(path, "rb");

		if (ddsFile)
		{
			DDS_DDSURFACEDESC2 ddsHeader;
			bool success = false;

			fseek(ddsFile, 4, SEEK_SET); // skip past tag
			fread(&ddsHeader, sizeof(ddsHeader), 1, ddsFile);

			if (ddsHeader.dwSize == sizeof(DDS_DDSURFACEDESC2))
			{
				w = (int)ddsHeader.dwWidth;
				h = (int)ddsHeader.dwHeight;

				if (out_format)
				{
					if ((ddsHeader.ddpfPixelFormat.dwFlags & DDS_DDPF_FOURCC) != 0 && ddsHeader.ddpfPixelFormat.dwFourCC == DDS_MAKE_MAGIC_NUMBER('D','X','1','0'))
					{
						DDS_HEADER_DXT10 dx10header;
						fread(&dx10header, sizeof(dx10header), 1, ddsFile);
						*out_format = (DDS_DXGI_FORMAT)dx10header.dxgiFormat;
					}
					else
					{
						*out_format = GetDX10FormatFromDDSPixelFormat(ddsHeader.ddpfPixelFormat);
					}
				}

				success = true;
			}

			fclose(ddsFile);
			return success;
		}
	}
#if defined(_FREEIMAGE)
	else
	{
		FreeImageInit();

		FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(path);

		if (fif == FIF_UNKNOWN)
		{
			fif = FreeImage_GetFIFFromFilename(path);
		}

		if (fif != FIF_UNKNOWN && FreeImage_FIFSupportsReading(fif))
		{
			FIBITMAP* dib = FreeImage_Load(fif, path, FIF_LOAD_NOPIXELS);

			if (dib)
			{
				w = FreeImage_GetWidth(dib);
				h = FreeImage_GetHeight(dib);

				if (out_format)
				{
					// approximate format from FreeImage .. 
					switch (FreeImage_GetImageType(dib))
					{
					case FIT_UNKNOWN : *out_format = DDS_DXGI_FORMAT_UNKNOWN; break;
					case FIT_UINT16  : *out_format = DDS_DXGI_FORMAT_R16_UINT; break;
					case FIT_INT16   : *out_format = DDS_DXGI_FORMAT_R16_SINT; break;
					case FIT_UINT32  : *out_format = DDS_DXGI_FORMAT_R32_UINT; break;
					case FIT_INT32   : *out_format = DDS_DXGI_FORMAT_R32_SINT; break;
					case FIT_FLOAT   : *out_format = DDS_DXGI_FORMAT_R32_FLOAT; break;
					case FIT_DOUBLE  : *out_format = DDS_DXGI_FORMAT_R32_FLOAT; break; // close enough
					case FIT_COMPLEX : *out_format = DDS_DXGI_FORMAT_R32G32_FLOAT; break; // close enough
					case FIT_RGB16   : *out_format = DDS_DXGI_FORMAT_R16G16B16A16_UNORM; break;
					case FIT_RGBA16  : *out_format = DDS_DXGI_FORMAT_R16G16B16A16_UNORM; break;
					case FIT_RGBF    : *out_format = DDS_DXGI_FORMAT_R32G32B32_FLOAT; break;
					case FIT_RGBAF   : *out_format = DDS_DXGI_FORMAT_R32G32B32A32_FLOAT; break;
					case FIT_BITMAP  :
						if      (FreeImage_GetBPP(dib) <=  8) *out_format = DDS_DXGI_FORMAT_R8_UNORM; // really "L8"
						else if (FreeImage_GetBPP(dib) <= 16) *out_format = DDS_DXGI_FORMAT_R16_UNORM; // ?
						else                                  *out_format = DDS_DXGI_FORMAT_B8G8R8A8_UNORM; // 24 or 32 bpp
						break;
					}
				}

				FreeImage_Unload(dib);
				return true;
			}
		}
	}
#endif // defined(_FREEIMAGE)

	w = 0;
	h = 0;
	return false;
}

template <typename DstType, typename SrcType> static DstType* LoadImageDDSInternal_T(const char* path, const void* src_, uint32 ddsSize, uint32 headerSize, int w, int h, int y0 = 0, bool redBlueSwap = false)
{
	DstType* dst = NULL;
	const uint32 expectedSize = headerSize + w*(y0 + h)*sizeof(SrcType);

	if (ddsSize >= expectedSize)
	{
		const SrcType* src = reinterpret_cast<const SrcType*>(src_) + w*y0;
		dst = new DstType[w*h];

		for (int i = 0; i < w*h; i++)
		{
			dst[i] = ConvertPixel<DstType,SrcType>(src[i]);

			if (redBlueSwap)
			{
				int pixelType = (int)GetPixelType<DstType>::T; // clang doesn't like it if i put this directly in the switch .. and i can't make this const!
				switch (pixelType) // really should only be doing this for Pixel32 when loading D3DFMT_A8B8G8R8 ..
				{
				case PixelType_Pixel32: break;
				case PixelType_Pixel64: std::swap(((Pixel64*)dst)[i].r, ((Pixel64*)dst)[i].b); break;
				case PixelType_Vec3f: std::swap(((Vec3f*)dst)[i].x_ref(), ((Vec3f*)dst)[i].z_ref()); break;
				case PixelType_Vec4V: ((Vec4V*)dst)[i] = Vec4V(_vmath_permute_ps<2,1,0,3>(((const Vec4V*)dst)[i])); break;
				}
			}
		}
	}
	else
	{
		fprintf(stderr, "LoadImageDDSInternal_T<%s,%s>: %u bytes short when loading %s\n", GetPixelType<DstType>::Str(), GetPixelType<SrcType>::Str(), expectedSize - ddsSize, path);
	}

	return dst;
}

template <typename DstType> static DstType* LoadImageDDS_T(const char* path, int& w, int& h)
{
	DstType* image = NULL;

	const char* ext = strrchr(path, '.');

	if (ext && stricmp(ext, ".dds") == 0)
	{
		FILE* ddsFile = fopen(path, "rb");

		if (ddsFile)
		{
			fseek(ddsFile, 0, SEEK_END);
			const uint32 ddsSize = (uint32)ftell(ddsFile);
			fseek(ddsFile, 0, SEEK_SET);

			if (ddsSize > sizeof(DDS_DDSURFACEDESC2))
			{
				uint8* ddsData = new uint8[ddsSize];
				fread(ddsData, ddsSize, 1, ddsFile);

				const DDS_DDSURFACEDESC2* ddsHeader = (DDS_DDSURFACEDESC2*)(ddsData + sizeof(uint32));
				const void*               ddsPixels = (const uint8*)ddsHeader + ddsHeader->dwSize;

				assert(ddsHeader->dwSize == sizeof(DDS_DDSURFACEDESC2));

				if ((w == 0 && h == 0) || (w == (int)ddsHeader->dwWidth && h == (int)ddsHeader->dwHeight))
				{
					w = (int)ddsHeader->dwWidth;
					h = (int)ddsHeader->dwHeight;

					DDS_D3DFORMAT format = GetD3DFormatFromDDSPixelFormat(ddsHeader->ddpfPixelFormat);
					int headerSize = sizeof(uint32) + sizeof(DDS_DDSURFACEDESC2);

					if (format == DDS_MAKE_MAGIC_NUMBER('D','X','1','0'))
					{
						const DDS_HEADER_DXT10* dx10header = (const DDS_HEADER_DXT10*)ddsPixels;
						ddsPixels = (uint8*)ddsPixels + sizeof(DDS_HEADER_DXT10);
						format = GetD3DFormatFromDDSPixelFormat(GetDDSPixelFormatFromDX10Format((DDS_DXGI_FORMAT)dx10header->dxgiFormat));
						headerSize += sizeof(DDS_HEADER_DXT10);
					}

					if      (format == DDS_D3DFMT_L8           ) image = LoadImageDDSInternal_T<DstType,uint8  >(path, ddsPixels, ddsSize, headerSize, w, h);
					else if (format == DDS_D3DFMT_L16          ) image = LoadImageDDSInternal_T<DstType,uint16 >(path, ddsPixels, ddsSize, headerSize, w, h);
					else if (format == DDS_D3DFMT_R32F         ) image = LoadImageDDSInternal_T<DstType,float  >(path, ddsPixels, ddsSize, headerSize, w, h);
					else if (format == DDS_D3DFMT_A32B32G32R32F) image = LoadImageDDSInternal_T<DstType,Vec4V  >(path, ddsPixels, ddsSize, headerSize, w, h);
					else if (format == DDS_D3DFMT_A8R8G8B8     ) image = LoadImageDDSInternal_T<DstType,Pixel32>(path, ddsPixels, ddsSize, headerSize, w, h);
					else if (format == DDS_D3DFMT_A8B8G8R8     ) image = LoadImageDDSInternal_T<DstType,Pixel32>(path, ddsPixels, ddsSize, headerSize, w, h, 0, true);
					else if (format == DDS_D3DFMT_A16B16G16R16 )
					{
						image = LoadImageDDSInternal_T<DstType,Pixel64>(path, ddsPixels, ddsSize, headerSize, w, h);

						// fix inconsistency when loading dds as RGBA16
						{
							Pixel64* pixels64 = reinterpret_cast<Pixel64*>(image);

							for (int i = 0; i < w*h; i++)
							{
								std::swap(pixels64[i].r, pixels64[i].b);
							}
						}
					}
					else
					{
						fprintf(stderr, "LoadImageDDS_T<%s>: unsupported format %s(%d) when loading %s\n", GetPixelType<DstType>::Str(), GetD3DFormatStr(format), format, path);
					}
				}
				else
				{
					fprintf(stderr, "LoadImageDDS_T<%s>: unexpected resolution %dx%d when loading %s, expected %dx%d\n", GetPixelType<DstType>::Str(), ddsHeader->dwWidth, ddsHeader->dwHeight, path, w, h);
				}

				delete[] ddsData;
			}

			fclose(ddsFile);
		}
	}

	return image;
}

#if defined(_FREEIMAGE)
template <typename DstType, typename SrcType> static DstType* LoadImageFreeImageInternal_T(FIBITMAP* dib, int w, int h, FREE_IMAGE_FORMAT fif)
{
	const bool flipped = FreeImageIsFlippedVertically(dib, fif);

	DstType* dst = new DstType[w*h];
	DstType* row = dst;

	for (int j = 0; j < h; j++)
	{
		const SrcType* pixels = reinterpret_cast<const SrcType*>(FreeImage_GetScanLine(dib, flipped ? j : (h - j - 1)));

		if (FreeImage_GetImageType(dib) == FIT_RGB16) // handle RGB16 specially, we don't have direct support for this type
		{
			StaticAssert(offsetof(Pixel64, b) < offsetof(Pixel64, r));
			struct Pixel48 { uint16 b, g, r; };

			for (int i = 0; i < w; i++)
			{
				Pixel64 pixel;
				pixel.r = (reinterpret_cast<const Pixel48*>(pixels))[i].r;
				pixel.g = (reinterpret_cast<const Pixel48*>(pixels))[i].g;
				pixel.b = (reinterpret_cast<const Pixel48*>(pixels))[i].b;
				pixel.a = 65535;

				// fix inconsistency when FreeImage loads png or tif as RGB16
				if (fif == FIF_PNG || fif == FIF_TIFF)
				{
					std::swap(pixel.r, pixel.b);
				}

				row[i] = ConvertPixel<DstType,Pixel64>(pixel);
			}
		}
		else if (FreeImage_GetImageType(dib) == FIT_RGBA16 && (fif == FIF_PNG || fif == FIF_TIFF))
		{
			// fix inconsistency when FreeImage loads png or tif as RGBA16
			for (int i = 0; i < w; i++)
			{
				Pixel64 pixel = reinterpret_cast<const Pixel64*>(pixels)[i];
				std::swap(pixel.r, pixel.b);
				row[i] = ConvertPixel<DstType,SrcType>(reinterpret_cast<const SrcType&>(pixel));
			}
		}
		else
		{
			for (int i = 0; i < w; i++)
			{
				row[i] = ConvertPixel<DstType,SrcType>(pixels[i]);
			}
		}

		row += w;
	}

	return dst;
}
#endif // defined(_FREEIMAGE)

template <typename DstType> DstType* LoadImage_T(const char* path, int& w, int& h TIFF_SAMPLE_INDEX_ONLY(, int sampleIndex))
{
	DstType* image = LoadImageDDS_T<DstType>(path, w, h);

	if (image)
	{
		return image;
	}

#if defined(_FREEIMAGE)
	FreeImageInit();

	FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(path);
	FIBITMAP* dib = NULL;

	if (fif == FIF_UNKNOWN)
	{
		fif = FreeImage_GetFIFFromFilename(path);
	}

	if (fif != FIF_UNKNOWN && FreeImage_FIFSupportsReading(fif))
	{
		int flags = 0;

#ifdef TIFF_SAMPLE_INDEX
		if (fif == FIF_TIFF && sampleIndex >= 0)
		{
			flags |= TIFF_LOAD_SAMPLE;
			flags |= (sampleIndex << TIFF_LOAD_SAMPLE_INDEX_SHIFT);
		}
#endif
		dib = FreeImage_Load(fif, path, flags);

		if (dib == NULL)
		{
			fprintf(stderr, "LoadImage_T<%s>: FreeImage_Load failed when loading %s\n", GetPixelType<DstType>::Str(), path);
			return NULL;
		}
	}
	else if (fif == FIF_UNKNOWN)
	{
		fprintf(stderr, "LoadImage_T<%s>: file format could not be derived when loading %s\n", GetPixelType<DstType>::Str(), path);
		return NULL;
	}
	else
	{
		const char* fifstr = "?";
		switch (fif)
		{
		case FIF_UNKNOWN : fifstr = "FIF_UNKNOWN"; break;
		case FIF_BMP     : fifstr = "FIF_BMP"    ; break;
		case FIF_ICO     : fifstr = "FIF_ICO"    ; break;
		case FIF_JPEG    : fifstr = "FIF_JPEG"   ; break;
		case FIF_JNG     : fifstr = "FIF_JNG"    ; break;
		case FIF_KOALA   : fifstr = "FIF_KOALA"  ; break;
		case FIF_LBM     : fifstr = "FIF_LBM"    ; break;
		case FIF_MNG     : fifstr = "FIF_MNG"    ; break;
		case FIF_PBM     : fifstr = "FIF_PBM"    ; break;
		case FIF_PBMRAW  : fifstr = "FIF_PBMRAW" ; break;
		case FIF_PCD     : fifstr = "FIF_PCD"    ; break;
		case FIF_PCX     : fifstr = "FIF_PCX"    ; break;
		case FIF_PGM     : fifstr = "FIF_PGM"    ; break;
		case FIF_PGMRAW  : fifstr = "FIF_PGMRAW" ; break;
		case FIF_PNG     : fifstr = "FIF_PNG"    ; break;
		case FIF_PPM     : fifstr = "FIF_PPM"    ; break;
		case FIF_PPMRAW  : fifstr = "FIF_PPMRAW" ; break;
		case FIF_RAS     : fifstr = "FIF_RAS"    ; break;
		case FIF_TARGA   : fifstr = "FIF_TARGA"  ; break;
		case FIF_TIFF    : fifstr = "FIF_TIFF"   ; break;
		case FIF_WBMP    : fifstr = "FIF_WBMP"   ; break;
		case FIF_PSD     : fifstr = "FIF_PSD"    ; break;
		case FIF_CUT     : fifstr = "FIF_CUT"    ; break;
		case FIF_XBM     : fifstr = "FIF_XBM"    ; break;
		case FIF_XPM     : fifstr = "FIF_XPM"    ; break;
		case FIF_DDS     : fifstr = "FIF_DDS"    ; break;
		case FIF_GIF     : fifstr = "FIF_GIF"    ; break;
		case FIF_HDR     : fifstr = "FIF_HDR"    ; break;
		case FIF_FAXG3   : fifstr = "FIF_FAXG3"  ; break;
		case FIF_SGI     : fifstr = "FIF_SGI"    ; break;
		case FIF_EXR     : fifstr = "FIF_EXR"    ; break;
		case FIF_J2K     : fifstr = "FIF_J2K"    ; break;
		case FIF_JP2     : fifstr = "FIF_JP2"    ; break;
		case FIF_PFM     : fifstr = "FIF_PFM"    ; break;
		case FIF_PICT    : fifstr = "FIF_PICT"   ; break;
		case FIF_RAW     : fifstr = "FIF_RAW"    ; break;
		}
		fprintf(stderr, "LoadImage_T<%s>: file format %s does not support reading when loading %s\n", GetPixelType<DstType>::Str(), fifstr, path);
		return NULL;
	}

	if ((w == 0 && h == 0) || (w == (int)FreeImage_GetWidth(dib) && h == (int)FreeImage_GetHeight(dib)))
	{
		w = FreeImage_GetWidth(dib);
		h = FreeImage_GetHeight(dib);

		const FREE_IMAGE_TYPE fit = FreeImage_GetImageType(dib);

		if (fit == FIT_BITMAP)
		{
			if (FreeImage_GetBPP(dib) == 8)
			{
				image = LoadImageFreeImageInternal_T<DstType,uint8>(dib, w, h, fif);
			}
			else
			{
				if (FreeImage_GetBPP(dib) != 32)
				{
					FIBITMAP* dib2 = FreeImage_ConvertTo32Bits(dib);
					FreeImage_Unload(dib);

					if (dib2)
					{
						dib = dib2;
					}
					else
					{
						fprintf(stderr, "LoadImage_T<%s>: FreeImage_ConvertTo32Bits failed when loading %s\n", GetPixelType<DstType>::Str(), path);
						return NULL;
					}
				}

				image = LoadImageFreeImageInternal_T<DstType,Pixel32>(dib, w, h, fif);
			}
		}
		else if (fit == FIT_UINT16                    ) image = LoadImageFreeImageInternal_T<DstType,PIXELTYPE_uint16 >(dib, w, h, fif);
		else if (fit == FIT_FLOAT                     ) image = LoadImageFreeImageInternal_T<DstType,PIXELTYPE_float  >(dib, w, h, fif);
		else if (fit == FIT_RGBF                      ) image = LoadImageFreeImageInternal_T<DstType,PIXELTYPE_Vec3f  >(dib, w, h, fif);
		else if (fit == FIT_RGBAF                     ) image = LoadImageFreeImageInternal_T<DstType,PIXELTYPE_Vec4V  >(dib, w, h, fif);
		else if (fit == FIT_RGBA16 || fit == FIT_RGB16) image = LoadImageFreeImageInternal_T<DstType,PIXELTYPE_Pixel64>(dib, w, h, fif);
		else
		{
			const char* fitstr = "?";
			switch (fit)
			{
			case FIT_UNKNOWN : fitstr = "FIT_UNKNOWN"; break;
			case FIT_BITMAP  : fitstr = "FIT_BITMAP" ; break;
			case FIT_UINT16  : fitstr = "FIT_UINT16" ; break;
			case FIT_INT16   : fitstr = "FIT_INT16"  ; break;
			case FIT_UINT32  : fitstr = "FIT_UINT32" ; break;
			case FIT_INT32   : fitstr = "FIT_INT32"  ; break;
			case FIT_FLOAT   : fitstr = "FIT_FLOAT"  ; break;
			case FIT_DOUBLE  : fitstr = "FIT_DOUBLE" ; break;
			case FIT_COMPLEX : fitstr = "FIT_COMPLEX"; break;
			case FIT_RGB16   : fitstr = "FIT_RGB16"  ; break;
			case FIT_RGBA16  : fitstr = "FIT_RGBA16" ; break;
			case FIT_RGBF    : fitstr = "FIT_RGBF"   ; break;
			case FIT_RGBAF   : fitstr = "FIT_RGBAF"  ; break;
			}

			fprintf(stderr, "LoadImage_T<%s>: unsupported image type %s when loading %s\n", GetPixelType<DstType>::Str(), fitstr, path);
		}

		FreeImage_Unload(dib);
	}
	else
	{
		// unexpected resolution
	}
#endif // defined(_FREEIMAGE)

	return image;
}

PIXELTYPE_uint8  * LoadImage_uint8  (const char* path, int& w, int& h TIFF_SAMPLE_INDEX_ONLY(, int sampleIndex)) { return LoadImage_T<PIXELTYPE_uint8  >(path, w, h TIFF_SAMPLE_INDEX_ONLY(, sampleIndex)); }
PIXELTYPE_uint16 * LoadImage_uint16 (const char* path, int& w, int& h TIFF_SAMPLE_INDEX_ONLY(, int sampleIndex)) { return LoadImage_T<PIXELTYPE_uint16 >(path, w, h TIFF_SAMPLE_INDEX_ONLY(, sampleIndex)); }
PIXELTYPE_float  * LoadImage_float  (const char* path, int& w, int& h TIFF_SAMPLE_INDEX_ONLY(, int sampleIndex)) { return LoadImage_T<PIXELTYPE_float  >(path, w, h TIFF_SAMPLE_INDEX_ONLY(, sampleIndex)); }
PIXELTYPE_Vec3f  * LoadImage_Vec3f  (const char* path, int& w, int& h TIFF_SAMPLE_INDEX_ONLY(, int sampleIndex)) { return LoadImage_T<PIXELTYPE_Vec3f  >(path, w, h TIFF_SAMPLE_INDEX_ONLY(, sampleIndex)); }
PIXELTYPE_Vec4V  * LoadImage_Vec4V  (const char* path, int& w, int& h TIFF_SAMPLE_INDEX_ONLY(, int sampleIndex)) { return LoadImage_T<PIXELTYPE_Vec4V  >(path, w, h TIFF_SAMPLE_INDEX_ONLY(, sampleIndex)); }
PIXELTYPE_Pixel32* LoadImage_Pixel32(const char* path, int& w, int& h TIFF_SAMPLE_INDEX_ONLY(, int sampleIndex)) { return LoadImage_T<PIXELTYPE_Pixel32>(path, w, h TIFF_SAMPLE_INDEX_ONLY(, sampleIndex)); }
PIXELTYPE_Pixel64* LoadImage_Pixel64(const char* path, int& w, int& h TIFF_SAMPLE_INDEX_ONLY(, int sampleIndex)) { return LoadImage_T<PIXELTYPE_Pixel64>(path, w, h TIFF_SAMPLE_INDEX_ONLY(, sampleIndex)); }

template <typename DstType> static DstType* LoadImageDDSSpan_T(const char* path, int w, int y0, int h)
{
	DstType* image = NULL;

	const char* ext = strrchr(path, '.');

	if (ext && stricmp(ext, ".dds") == 0)
	{
		FILE* ddsFile = fopen(path, "rb");

		if (ddsFile)
		{
			fseek(ddsFile, 0, SEEK_END);
			const uint32 ddsSize = (uint32)ftell(ddsFile);
			fseek(ddsFile, 0, SEEK_SET);

			if (ddsSize > sizeof(DDS_DDSURFACEDESC2))
			{
				uint8* ddsData = new uint8[ddsSize];
				fread(ddsData, ddsSize, 1, ddsFile);

				const DDS_DDSURFACEDESC2* ddsHeader = (DDS_DDSURFACEDESC2*)(ddsData + sizeof(uint32));
				const void*           ddsPixels = (const uint8*)ddsHeader + ddsHeader->dwSize;

				assert(ddsHeader->dwSize == sizeof(DDS_DDSURFACEDESC2));

				if (w == (int)ddsHeader->dwWidth && y0 >= 0 && y0 + h <= (int)ddsHeader->dwHeight)
				{
					w = (int)ddsHeader->dwWidth;
					h = (int)ddsHeader->dwHeight;

					DDS_D3DFORMAT format = GetD3DFormatFromDDSPixelFormat(ddsHeader->ddpfPixelFormat);
					int headerSize = sizeof(uint32) + sizeof(DDS_DDSURFACEDESC2);

					if (format == DDS_MAKE_MAGIC_NUMBER('D','X','1','0'))
					{
						const DDS_HEADER_DXT10* dx10header = (const DDS_HEADER_DXT10*)ddsPixels;
						ddsPixels = (uint8*)ddsPixels + sizeof(DDS_HEADER_DXT10);
						format = GetD3DFormatFromDDSPixelFormat(GetDDSPixelFormatFromDX10Format((DDS_DXGI_FORMAT)dx10header->dxgiFormat));
						headerSize += sizeof(DDS_HEADER_DXT10);
					}

					if      (format == DDS_D3DFMT_L8           ) image = LoadImageDDSInternal_T<DstType,PIXELTYPE_uint8  >(path, ddsPixels, ddsSize, headerSize, w, h, y0);
					else if (format == DDS_D3DFMT_L16          ) image = LoadImageDDSInternal_T<DstType,PIXELTYPE_uint16 >(path, ddsPixels, ddsSize, headerSize, w, h, y0);
					else if (format == DDS_D3DFMT_R32F         ) image = LoadImageDDSInternal_T<DstType,PIXELTYPE_float  >(path, ddsPixels, ddsSize, headerSize, w, h, y0);
					else if (format == DDS_D3DFMT_A32B32G32R32F) image = LoadImageDDSInternal_T<DstType,PIXELTYPE_Vec4V  >(path, ddsPixels, ddsSize, headerSize, w, h, y0);
					else if (format == DDS_D3DFMT_A8R8G8B8     ) image = LoadImageDDSInternal_T<DstType,PIXELTYPE_Pixel32>(path, ddsPixels, ddsSize, headerSize, w, h, y0);
					else if (format == DDS_D3DFMT_A8B8G8R8     ) image = LoadImageDDSInternal_T<DstType,PIXELTYPE_Pixel32>(path, ddsPixels, ddsSize, headerSize, w, h, y0, true);
					else if (format == DDS_D3DFMT_A16B16G16R16 ) image = LoadImageDDSInternal_T<DstType,PIXELTYPE_Pixel64>(path, ddsPixels, ddsSize, headerSize, w, h, y0);
					else
					{
						fprintf(stderr, "LoadImageDDSSpan_T<%s>: unsupported format %s(%d) when loading %s\n", GetPixelType<DstType>::Str(), GetD3DFormatStr(format), format, path);
					}
				}
				else
				{
					fprintf(stderr, "LoadImageDDSSpan_T<%s>: unexpected resolution %dx%d when loading %s, expected %dx%d (or taller)\n", GetPixelType<DstType>::Str(), ddsHeader->dwWidth, ddsHeader->dwHeight, path, w, y0 + h);
				}

				delete[] ddsData;
			}

			fclose(ddsFile);
		}
	}

	return image;
}

PIXELTYPE_uint8  * LoadImageDDSSpan_uint8  (const char* path, int w, int y0, int h) { return LoadImageDDSSpan_T<PIXELTYPE_uint8  >(path, w, y0, h); }
PIXELTYPE_uint16 * LoadImageDDSSpan_uint16 (const char* path, int w, int y0, int h) { return LoadImageDDSSpan_T<PIXELTYPE_uint16 >(path, w, y0, h); }
PIXELTYPE_float  * LoadImageDDSSpan_float  (const char* path, int w, int y0, int h) { return LoadImageDDSSpan_T<PIXELTYPE_float  >(path, w, y0, h); }
PIXELTYPE_Vec3f  * LoadImageDDSSpan_Vec3f  (const char* path, int w, int y0, int h) { return LoadImageDDSSpan_T<PIXELTYPE_Vec3f  >(path, w, y0, h); }
PIXELTYPE_Vec4V  * LoadImageDDSSpan_Vec4f  (const char* path, int w, int y0, int h) { return LoadImageDDSSpan_T<PIXELTYPE_Vec4V  >(path, w, y0, h); }
PIXELTYPE_Pixel32* LoadImageDDSSpan_Pixel32(const char* path, int w, int y0, int h) { return LoadImageDDSSpan_T<PIXELTYPE_Pixel32>(path, w, y0, h); }
PIXELTYPE_Pixel64* LoadImageDDSSpan_Pixel64(const char* path, int w, int y0, int h) { return LoadImageDDSSpan_T<PIXELTYPE_Pixel64>(path, w, y0, h); }

template <typename DstType> static DDS_DDPIXELFORMAT GetDDSFormatInternal_T();

template <> DDS_DDPIXELFORMAT GetDDSFormatInternal_T<uint8>()
{
	DDS_DDPIXELFORMAT ddpf;
	memset(&ddpf, 0, sizeof(ddpf));
	ddpf.dwSize = sizeof(ddpf);
	ddpf.dwFlags = DDS_DDPF_LUMINANCE;
	ddpf.dwRGBBitCount = 8;
	ddpf.dwRBitMask = 0x000000ff;
	return ddpf;
}

template <> DDS_DDPIXELFORMAT GetDDSFormatInternal_T<uint16>()
{
	DDS_DDPIXELFORMAT ddpf;
	memset(&ddpf, 0, sizeof(ddpf));
	ddpf.dwSize = sizeof(ddpf);
	ddpf.dwFlags = DDS_DDPF_LUMINANCE;
	ddpf.dwRGBBitCount = 16;
	ddpf.dwRBitMask = 0x0000ffff;
	return ddpf;
}

template <> DDS_DDPIXELFORMAT GetDDSFormatInternal_T<float>()
{
	DDS_DDPIXELFORMAT ddpf;
	memset(&ddpf, 0, sizeof(ddpf));
	ddpf.dwSize = sizeof(ddpf);
	ddpf.dwFlags = DDS_DDPF_FOURCC;
	ddpf.dwFourCC = DDS_D3DFMT_R32F;
	return ddpf;
}

template <> DDS_DDPIXELFORMAT GetDDSFormatInternal_T<Vec3f>(); // not supported

template <> DDS_DDPIXELFORMAT GetDDSFormatInternal_T<Vec4V>()
{
	DDS_DDPIXELFORMAT ddpf;
	memset(&ddpf, 0, sizeof(ddpf));
	ddpf.dwSize = sizeof(ddpf);
	ddpf.dwFlags = DDS_DDPF_FOURCC;
	ddpf.dwFourCC = DDS_D3DFMT_A32B32G32R32F;
	return ddpf;
}

template <> DDS_DDPIXELFORMAT GetDDSFormatInternal_T<Pixel32>()
{
	DDS_DDPIXELFORMAT ddpf;
	memset(&ddpf, 0, sizeof(ddpf));
	ddpf.dwSize = sizeof(ddpf);
	ddpf.dwFlags = DDS_DDPF_RGBA;
	ddpf.dwRGBBitCount = 32;
	ddpf.dwRBitMask = 0x00ff0000;
	ddpf.dwGBitMask = 0x0000ff00;
	ddpf.dwBBitMask = 0x000000ff;
	ddpf.dwABitMask = 0xff000000;
	return ddpf;
}

template <> DDS_DDPIXELFORMAT GetDDSFormatInternal_T<Pixel64>()
{
	DDS_DDPIXELFORMAT ddpf;
	memset(&ddpf, 0, sizeof(ddpf));
	ddpf.dwSize = sizeof(ddpf);
	ddpf.dwFlags = DDS_DDPF_FOURCC;
	ddpf.dwFourCC = DDS_D3DFMT_A16B16G16R16;
	return ddpf;
}

template <typename DstType, typename SrcType> static bool SaveImageDDS_T(const char* path, const SrcType* image, int w, int h)
{
	const char* ext = strrchr(path, '.');

	if (ext && stricmp(ext, ".dds") == 0)
	{
		FILE* dds = fopen(path, "wb");

		if (dds)
		{
			WriteDDSHeader(dds, DDS_IMAGE_TYPE_2D, w, h, 1, 1, 1, GetDDSFormatInternal_T<DstType>());

			// fix inconsistency when saving dds as RGBA16
			const bool bSwapRBForPixel64 = (GetPixelType<DstType>::T == PixelType_Pixel64);

			if (GetPixelType<DstType>::T == GetPixelType<SrcType>::T && !bSwapRBForPixel64)
			{
				fwrite(image, sizeof(DstType), w*h, dds);
			}
			else
			{
				DstType* pixels = new DstType[w];

				for (int j = 0; j < h; j++)
				{
					for (int i = 0; i < w; i++)
					{
						pixels[i] = ConvertPixel<DstType,SrcType>(image[i]);
					}

					if (bSwapRBForPixel64)
					{
						Pixel64* pixels64 = reinterpret_cast<Pixel64*>(pixels);

						for (int i = 0; i < w; i++)
						{
							std::swap(pixels64[i].r, pixels64[i].b);
						}
					}

					fwrite(pixels, sizeof(DstType), w, dds);
					image += w; // next row
				}

				delete[] pixels;
			}

			fclose(dds);
			return true;
		}
	}

	return false;
}

#if defined(_FREEIMAGE)
template <typename DstType> static FIBITMAP* AllocImageFreeImageInternal_T(int w, int h, FREE_IMAGE_FORMAT fif);

template <> FIBITMAP* AllocImageFreeImageInternal_T<uint8>(int w, int h, FREE_IMAGE_FORMAT)
{
	return FreeImage_Allocate(w, h, 8);
}

template <> FIBITMAP* AllocImageFreeImageInternal_T<uint16>(int w, int h, FREE_IMAGE_FORMAT fif)
{
	if (FreeImage_FIFSupportsExportType(fif, FIT_UINT16))
	{
		return FreeImage_AllocateT(FIT_UINT16, w, h, 16);
	}
	else
	{
		return NULL;
	}
}

template <> FIBITMAP* AllocImageFreeImageInternal_T<float>(int w, int h, FREE_IMAGE_FORMAT fif)
{
	if (FreeImage_FIFSupportsExportType(fif, FIT_FLOAT))
	{
		return FreeImage_AllocateT(FIT_FLOAT, w, h, 32);
	}
	else
	{
		return NULL;
	}
}

template <> FIBITMAP* AllocImageFreeImageInternal_T<Vec3f>(int w, int h, FREE_IMAGE_FORMAT fif)
{
	if (FreeImage_FIFSupportsExportType(fif, FIT_RGBF))
	{
		return FreeImage_AllocateT(FIT_RGBF, w, h, 96);
	}
	else
	{
		return NULL;
	}
}

template <> FIBITMAP* AllocImageFreeImageInternal_T<Vec4V>(int w, int h, FREE_IMAGE_FORMAT fif)
{
	if (FreeImage_FIFSupportsExportType(fif, FIT_RGBAF))
	{
		return FreeImage_AllocateT(FIT_RGBAF, w, h, 128);
	}
	else
	{
		return NULL;
	}
}

class Pixel24 // required for saving color images as jpg
{
public:
	Pixel24() {}
	Pixel24(int r_,int g_,int b_): b((uint8)b_),g((uint8)g_),r((uint8)r_) {}

	uint8 b,g,r;
};

template <> const Pixel24 ConvertPixel<Pixel24,uint8  >(const uint8  & p) { return Pixel24(p,p,p); }
template <> const Pixel24 ConvertPixel<Pixel24,uint16 >(const uint16 & p) { const uint8 q = ConvertPixel<uint8,uint16>(p); return Pixel24(q,q,q); }
template <> const Pixel24 ConvertPixel<Pixel24,float  >(const float  & p) { const uint8 q = ConvertPixel<uint8,float>(p); return Pixel24(q,q,q); }
template <> const Pixel24 ConvertPixel<Pixel24,Vec3f  >(const Vec3f  & p) { return Pixel24(ConvertPixel<uint8,float>(p.x()),ConvertPixel<uint8,float>(p.y()),ConvertPixel<uint8,float>(p.z())); }
template <> const Pixel24 ConvertPixel<Pixel24,Vec4V  >(const Vec4V  & p) { return Pixel24(ConvertPixel<uint8,float>(p.xf()),ConvertPixel<uint8,float>(p.yf()),ConvertPixel<uint8,float>(p.zf())); }
template <> const Pixel24 ConvertPixel<Pixel24,Pixel32>(const Pixel32& p) { return Pixel24(p.r,p.g,p.b); }
template <> const Pixel24 ConvertPixel<Pixel24,Pixel64>(const Pixel64& p) { return Pixel24(ConvertPixel<uint8,uint16>(p.r),ConvertPixel<uint8,uint16>(p.g),ConvertPixel<uint8,uint16>(p.b)); }

template <> FIBITMAP* AllocImageFreeImageInternal_T<Pixel24>(int w, int h, FREE_IMAGE_FORMAT)
{
	return FreeImage_Allocate(w, h, 24);
}

template <> FIBITMAP* AllocImageFreeImageInternal_T<Pixel32>(int w, int h, FREE_IMAGE_FORMAT)
{
	return FreeImage_Allocate(w, h, 32);
}

template <> FIBITMAP* AllocImageFreeImageInternal_T<Pixel64>(int w, int h, FREE_IMAGE_FORMAT fif)
{
	if (FreeImage_FIFSupportsExportType(fif, FIT_RGBA16))
	{
		return FreeImage_AllocateT(FIT_RGBA16, w, h, 64);
	}
	else
	{
		return NULL;
	}
}

template <typename DstType, typename SrcType> static bool SaveImageFreeImageInternal_T(const char* path, const SrcType* image, int w, int h, FREE_IMAGE_FORMAT fif)
{
	FIBITMAP* dib = AllocImageFreeImageInternal_T<DstType>(w, h, fif);
	bool success = false;

	if (dib)
	{
		for (int j = 0; j < h; j++)
		{
			DstType* pixels = reinterpret_cast<DstType*>(FreeImage_GetScanLine(dib, h - j - 1));

			for (int i = 0; i < w; i++)
			{
				pixels[i] = ConvertPixel<DstType,SrcType>(image[i]);
			}

			// fix inconsistency when FreeImage saves png or tif as RGBA16
			if (GetPixelType<DstType>::T == PixelType_Pixel64 && (fif == FIF_PNG || fif == FIF_TIFF))
			{
				Pixel64* pixels64 = reinterpret_cast<Pixel64*>(pixels);

				for (int i = 0; i < w; i++)
				{
					std::swap(pixels64[i].r, pixels64[i].b);
				}
			}

			image += w; // next row
		}

		success = FreeImage_Save(fif, dib, path) != 0;

		FreeImage_Unload(dib);
	}

	return success;
}
#endif // defined(_FREEIMAGE)

template <typename DstType, typename SrcType> static bool SaveImage_T(const char* path, const SrcType* image, int w, int h, bool bAutoOpen, bool bNative)
{
	bool success = SaveImageDDS_T<DstType,SrcType>(path, image, w, h);

#if defined(_FREEIMAGE)
	if (!success)
	{
		FreeImageInit();

		const FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(path);

		if (fif == FIF_UNKNOWN)
		{
			return false; // unknown filetype
		}

		if (!FreeImage_FIFSupportsWriting(fif))
		{
			return false; // cannot save to this filetype
		}

		if (bNative)
		{
			success = SaveImageFreeImageInternal_T<DstType,SrcType>(path, image, w, h, fif);
		}

		if (!success)
		{
			if (GetPixelType<SrcType>::N > 1)
			{
				success = SaveImageFreeImageInternal_T<Pixel32,SrcType>(path, image, w, h, fif);

				if (!success) // maybe it's a format which doesn't support alpha, like jpg
				{
					success = SaveImageFreeImageInternal_T<Pixel24,SrcType>(path, image, w, h, fif);
				}
			}
			else // scalar->greyscale
			{
				success = SaveImageFreeImageInternal_T<uint8,SrcType>(path, image, w, h, fif);
			}
		}
	}
#else
	(void)bNative;
#endif

	if (success && bAutoOpen)
	{
	#if PLATFORM_PC
		system(path);
	#endif // PLATFORM_PC
	}

	return success;
}

bool SaveImage(const char* path, const PIXELTYPE_uint8  * image, int w, int h, bool bAutoOpen, bool bNative) { return SaveImage_T<uint8  ,PIXELTYPE_uint8  >(path, image, w, h, bAutoOpen, bNative); }
bool SaveImage(const char* path, const PIXELTYPE_uint16 * image, int w, int h, bool bAutoOpen, bool bNative) { return SaveImage_T<uint16 ,PIXELTYPE_uint16 >(path, image, w, h, bAutoOpen, bNative); }
bool SaveImage(const char* path, const PIXELTYPE_float  * image, int w, int h, bool bAutoOpen, bool bNative) { return SaveImage_T<float  ,PIXELTYPE_float  >(path, image, w, h, bAutoOpen, bNative); }
bool SaveImage(const char* path, const PIXELTYPE_Vec3f  * image, int w, int h, bool bAutoOpen, bool bNative) { return SaveImage_T<Vec4V  ,PIXELTYPE_Vec3f  >(path, image, w, h, bAutoOpen, bNative); } // converts to Vec4f
bool SaveImage(const char* path, const PIXELTYPE_Vec4V  * image, int w, int h, bool bAutoOpen, bool bNative) { return SaveImage_T<Vec4V  ,PIXELTYPE_Vec4V  >(path, image, w, h, bAutoOpen, bNative); }
bool SaveImage(const char* path, const PIXELTYPE_Pixel32* image, int w, int h, bool bAutoOpen, bool bNative) { return SaveImage_T<Pixel32,PIXELTYPE_Pixel32>(path, image, w, h, bAutoOpen, bNative); }
bool SaveImage(const char* path, const PIXELTYPE_Pixel64* image, int w, int h, bool bAutoOpen, bool bNative) { return SaveImage_T<Pixel64,PIXELTYPE_Pixel64>(path, image, w, h, bAutoOpen, bNative); }

Pixel32 GetUniqueColor(uint32 index)
{
	// https://www.mathworks.com/matlabcentral/mlc-downloads/downloads/submissions/29702/versions/3/screenshot.png
	const Pixel32 colors[] = {
		Pixel32(255,0,0),
		Pixel32(0,255,0),
		Pixel32(0,0,255),
		Pixel32(0,0,43),
		Pixel32(255,26,144),
		Pixel32(255,211,0),
		Pixel32(0,87,0),
		Pixel32(131,131,255),
		Pixel32(158,79,70),
		Pixel32(0,255,193),
		Pixel32(0,131,149),
		Pixel32(0,0,123),
		Pixel32(149,211,79),
		Pixel32(246,158,219),
		Pixel32(211,17,255),
		Pixel32(123,26,105),
		Pixel32(246,17,96),
		Pixel32(255,193,131),
		Pixel32(35,35,8),
		Pixel32(140,167,123),
		Pixel32(246,131,8),
		Pixel32(131,114,0),
		Pixel32(114,246,255),
		Pixel32(158,193,255),
		Pixel32(114,96,123),
	};
	return colors[index%countof(colors)];
}

#if 0
// given an array of w floats in 0-1 range, build an antialiased "graph" image of dimensions (w,h)
float* Graph(float* dst, const float* src, int w, int h)
{
	if (h == 0)
	{
		h = w/2;
	}

	for (int x = 0; x < w; x++)
	{
		float v0 = Clamp(1.0f - src[Max(0, x - 1)], 0.0f, 1.0f - 0.00001f)*(float)h; // [0..h]
		float v1 = Clamp(1.0f - src[Min(x, w - 1)], 0.0f, 1.0f - 0.00001f)*(float)h; // [0..h]
		float h0 = Min(v0, v1);
		float h1 = Max(v0, v1);
		float h0_floor = floorf(h0);

		int ymin = Clamp((int)floorf(h0), 0, h - 1);
		int ymax = Clamp((int)floorf(h1), 0, h - 1) + 1;

		float* p = &dst[x]; // pointer to column data (dst)

		for (int y = 0; y < ymin; y++) // over
		{
			*p = 1.0f, p += w;
		}

		if (ymax - ymin == 1)
		{
			const float coverage = (h1 + h0)/2.0f - h0_floor;

			*p = coverage, p += w;
		}
		else
		{
			float y0 = h0_floor + 0.0f;
			float y1 = h0_floor + 1.0f;
			float dx = 1.0f/(h1 - h0);
			float x0 = (y0 - h0)*dx;
			float x1 = (y1 - h0)*dx;

			for (int y = ymin; y < ymax; y++)
			{
				// compute coverage for pixel (x,y) - "bounds" of pixel is [0..1],[y..y+1]
				// intersect with line from (0,h0) to (1,h1)

				float coverage = 1.0f - (x1 + x0)/2.0f;

				coverage -= Max(0.0f, -x0       )*(h0 - y0)/2.0f;
				coverage -= Max(0.0f,  x1 - 1.0f)*(h1 - y1)/2.0f;

				*p = coverage, p += w;

				y0 = y1; y1 += 1.0f;
				x0 = x1; x1 += dx;
			}
		}

		for (int y = ymax; y < h; y++) // under
		{
			*p = 0.0f, p += w;
		}
	}

	return dst;
}
#endif

// given an array of w floats in 0-1 range, build an antialiased "graph" image of dimensions (w,h)
template <typename T> static float* ImageGraph_T(float* dst, const T* src, int w, int h)
{
	if (h == 0)
		h = w/2;
	if (dst == NULL)
		dst = new float[w*h];
	for (int x = 0; x < w; x++) {
		const T v0 = Clamp(T(1) - src[Max(0, x - 1)], T(0), T(1.0 - 0.00001))*T(h); // [0..h]
		const T v1 = Clamp(T(1) - src[Min(x, w - 1)], T(0), T(1.0 - 0.00001))*T(h); // [0..h]
		const T h0 = Min(v0, v1);
		const T h1 = Max(v0, v1);
		const T h0_floor = Floor(h0);
		const int ymin = Clamp((int)Floor(h0), 0, h - 1);
		const int ymax = Clamp((int)Floor(h1), 0, h - 1) + 1;
		float* p = &dst[x]; // pointer to column data (dst)
		for (int y = 0; y < ymin; y++) // over
			*p = 1.0f, p += w;
		if (ymax - ymin == 1) {
			const T coverage = (h1 + h0)*T(0.5) - h0_floor;
			*p = (float)coverage, p += w;
		} else {
			const T dx = T(1)/(h1 - h0);
			T y0 = h0_floor;
			T y1 = h0_floor + T(1);
			T x0 = (y0 - h0)*dx;
			T x1 = (y1 - h0)*dx;
			for (int y = ymin; y < ymax; y++) {
				// compute coverage for pixel (x,y) - "bounds" of pixel is [0..1],[y..y+1]
				// intersect with line from (0,h0) to (1,h1)
				T coverage = T(1) - (x1 + x0)*T(0.5);
				coverage -= Max(T(0), -x0       )*(h0 - y0)*T(0.5);
				coverage -= Max(T(0), +x1 - T(1))*(h1 - y1)*T(0.5);
				*p = (float)coverage, p += w;
				y0 = y1; y1 += T(1);
				x0 = x1; x1 += dx;
			}
		}
		for (int y = ymax; y < h; y++) // under
			*p = 0.0f, p += w;
	}
	return dst;
}

float* ImageGraph(float* dst, const float* src, int w, int h) { return ImageGraph_T(dst, src, w, h); }
float* ImageGraph(float* dst, const double* src, int w, int h) { return ImageGraph_T(dst, src, w, h); }

class GraphSegment
{
public:
	GraphSegment() {}
	GraphSegment(int i0, int j0, int i1, int j1, Vec2V_arg ep0, Vec2V_arg ep1, float thickness)
		: m_i0(i0)
		, m_j0(j0)
		, m_i1(i1)
		, m_j1(j1)
		, m_ep0(ep0)
		, m_ep1(ep1)
		, m_thicknessSqr(thickness*thickness)
	{
		m_n = Normalize(ep1 - ep0);
		m_m = Mag(ep1 - ep0);
	}

	inline bool Test(Vec2V_arg p) const
	{
		// TODO -- optimize!
		const Vec2V v = p - m_ep0;
		return MagSqr(m_n*Clamp(Dot(v, m_n), ScalarV(V_ZERO), m_m) - v) <= m_thicknessSqr;
	}

	int m_i0;
	int m_j0;
	int m_i1;
	int m_j1;
	Vec2V m_ep0;
	Vec2V m_ep1;
	Vec2V m_n;
	ScalarV m_m;
	float m_thicknessSqr;
};

float* ImageGraph(float* dst, int w, int h, const float* graph, float rangeMin, float rangeMax, float thickness)
{
	if (h == 0)
		h = w/2;
	if (dst == NULL)
		dst = new float[w*h];
	if (thickness == -1.0f) {
		for (int x = 0; x < w; x++) {
			const float g0 = (graph[Max(0, x - 1)] - rangeMin)/(rangeMax - rangeMin);
			const float g1 = (graph[Min(x, w - 1)] - rangeMin)/(rangeMax - rangeMin);
			const float v0 = Clamp(1.0f - g0, 0.0f, (1.0f - 0.00001f))*float(h); // [0..h]
			const float v1 = Clamp(1.0f - g1, 0.0f, (1.0f - 0.00001f))*float(h); // [0..h]
			const float h0 = Min(v0, v1);
			const float h1 = Max(v0, v1);
			const float h0_floor = Floor(h0);
			const int ymin = Clamp((int)Floor(h0), 0, h - 1);
			const int ymax = Clamp((int)Floor(h1), 0, h - 1) + 1;
			float* p = &dst[x]; // pointer to column data (dst)
			for (int y = 0; y < ymin; y++) // over
				*p = 1.0f, p += w;
			if (ymax - ymin == 1) {
				const float coverage = (h1 + h0)*0.5f - h0_floor;
				*p = (float)coverage, p += w;
			} else {
				const float dx = 1.0f/(h1 - h0);
				float y0 = h0_floor;
				float y1 = h0_floor + 1.0f;
				float x0 = (y0 - h0)*dx;
				float x1 = (y1 - h0)*dx;
				for (int y = ymin; y < ymax; y++) {
					// compute coverage for pixel (x,y) - "bounds" of pixel is [0..1],[y..y+1]
					// intersect with line from (0,h0) to (1,h1)
					float coverage = 1.0f - (x1 + x0)*0.5f;
					coverage -= Max(0.0f, -x0       )*(h0 - y0)*0.5f;
					coverage -= Max(0.0f, +x1 - 1.0f)*(h1 - y1)*0.5f;
					*p = (float)coverage, p += w;
					y0 = y1; y1 += 1.0f;
					x0 = x1; x1 += dx;
				}
			}
			for (int y = ymax; y < h; y++) // under
				*p = 0.0f, p += w;
		}
		return dst;
	}
	const int supersample = 8;
	GraphSegment* segs = new GraphSegment[w];
	int* remap = new int[w*h];
	memset(remap, 0xFF, w*h*sizeof(int));
	int cells = 0;
	for (int n = 0; n < w; n++) {
		const float z0 = (graph[Max(0, n - 1)] - rangeMin)/(rangeMax - rangeMin);
		const float z1 = (graph[n]             - rangeMin)/(rangeMax - rangeMin);
		const int i0 = Max(0, n - (int)Ceiling(thickness));
		const int j0 = Max(0, (int)Floor((float)h*Min(z0, z1) - thickness));
		const int i1 = Min(n + (int)Ceiling(thickness) + 1, w - 1);
		const int j1 = Min((int)Floor((float)h*Max(z0, z1) + thickness) + 1, h - 1);
		const float x0 = (float)n;
		const float y0 = (float)h*z0;
		const float x1 = (float)n + 1.0f;
		const float y1 = (float)h*z1;
		segs[n] = GraphSegment(i0, j0, i1, j1, Vec2V(x0, y0), Vec2V(x1, y1), thickness);
		for (int j = j0; j < j1; j++) {
			for (int i = i0; i < i1; i++) {
				if (remap[i + j*w] == (int)-1) // only once per pixel
					remap[i + j*w] = cells++;
			}
		}
	}
	int bitCount = cells*supersample*supersample;
	int flagsSizeInBytes = (bitCount + 7)/8;
	uint8* flags = new uint8[flagsSizeInBytes];
	memset(flags, 0, flagsSizeInBytes);
	for (int n = 0; n < w; n++) {
		for (int j = segs[n].m_j0; j < segs[n].m_j1; j++) {
			for (int i = segs[n].m_i0; i < segs[n].m_i1; i++) {
				ForceAssert(i >= 0 && i < w && j >= 0 && j < h);
				for (int ssj = 0; ssj < supersample; ssj++) { // in this cell we need to set appropriate flags by testing distance to line segments ..
					for (int ssi = 0; ssi < supersample; ssi++) {
						const float x = (float)i + (0.5f + (float)ssi)/(float)supersample;
						const float y = (float)j + (0.5f + (float)ssj)/(float)supersample;
						if (segs[n].Test(Vec2V(x, y))) {
							const int bitIndex = ssi + (ssj + remap[i + j*w]*supersample)*supersample;
							flags[bitIndex>>3] |= BIT(bitIndex&7);
						}
					}
				}
			}
		}
	}
	for (int i = 0; i < w*h; i++)
		dst[i] = 1.0f;
	for (int n = 0; n < w; n++) {
		for (int j = segs[n].m_j0; j < segs[n].m_j1; j++) {
			for (int i = segs[n].m_i0; i < segs[n].m_i1; i++) {
				int count = 0;
				for (int k = 0; k < supersample*supersample; k++) {
					const int bitIndex = k + remap[i + j*w]*supersample*supersample;
					if (flags[bitIndex>>3] & BIT(bitIndex&7))
						count++;
				}
				dst[i + (h - 1 - j)*w] = 1.0f - (float)count/(float)(supersample*supersample);
			}
		}
	}
	delete[] remap;
	delete[] segs;
	return dst;
}

void ImageGraphOverlay(Vec4V* image, int w, int h, const float* graph, float rangeMin, float rangeMax, float thickness, Vec3V_arg color, float opacity)
{
	if (opacity > 0.0f) {
		float* g = ImageGraph(NULL, w, h, graph, rangeMin, rangeMax, thickness);
		for (int n = 0; n < w*h; n++) {
			if (g[n] < 1.0f)
				image[n] += (Vec4V(color,1.0f) - image[n])*(1.0f - g[n])*opacity;
		}
		delete[] g;
	}
}

Vec4V* ImageGraphSet(int w, int h, const std::vector<const std::vector<float>*>& graphs, float thickness)
{
	Vec4V* image = new Vec4V[w*h];
	for (int n = 0; n < w*h; n++)
		image[n] = Vec4V(V_ONE);
	float rangeMin = +FLT_MAX;
	float rangeMax = -FLT_MAX;
	for (uint32 j = 0; j < graphs.size(); j++) {
		const std::vector<float>* graph = graphs[j];
		if (graph) {
			for (uint32 i = 0; i < graph->size(); i++) {
				const float v = graph->operator[](i);
				rangeMin = Min(v, rangeMin);
				rangeMax = Max(v, rangeMax);
			}
		}
	}
	if (rangeMin < rangeMax) {
		
	}
	return image;
}

template <typename T> uint8* ImageHistogram_T(const T* data, size_t count, uint32 w, uint32 h, uint32 numBuckets, T* minValue_inout, T* maxValue_inout, float scale)
{
	uint8* image = new uint8[w*h];
	memset(image, 0, w*h*sizeof(uint8));
	T minValue = minValue_inout ? *minValue_inout : 0;
	T maxValue = maxValue_inout ? *maxValue_inout : 0;
	if (minValue >= maxValue) {
		minValue = +FLT_MAX;
		maxValue = -FLT_MAX;
		for (int i = 0; i < count; i++) {
			minValue = Min(data[i], minValue);
			maxValue = Max(data[i], maxValue);
		}
		if (minValue_inout)
			*minValue_inout = minValue;
		if (maxValue_inout)
			*maxValue_inout = maxValue;
	}
	if (minValue < maxValue) {
		uint32* hist = new uint32[numBuckets];
		memset(hist, 0, numBuckets*sizeof(uint32));
		const T n = T(numBuckets);
		for (uint32 i = 0; i < count; i++) {
			const T x = (data[i] - minValue)/(maxValue - minValue); // [0..1]
			if (x >= 0 && x <= 1) {
				const int b = Clamp((int)Floor(x*n), 0, numBuckets - 1); // [0..numBuckets-1]
				hist[b]++;
			}
		}
		uint32 maxHist = 0;
		for (uint32 i = 0; i < numBuckets; i++)
			maxHist = Max(hist[i], maxHist);
		uint32 b0 = numBuckets;
		for (uint32 i = 0; i < w; i++) {
			const float x = (float)i/(float)w; // [0..1)
			const uint32 b = (uint32)(x*n); // [0..numBuckets-1]
			const float y = (float)hist[b]/(float)maxHist; // [0..1]
			uint32 j1 = (uint32)(scale*y*(float)h); // [0..h]
			if (b0 != b) {
				b0 = b;
				j1 = 0;
			}
			for (uint32 j = j1; j < h; j++)
				image[i + (h - 1 - j)*w] = 0xFF;
		}
		delete[] hist;
	}
	return image;
}

uint8* ImageHistogram(const float* data, size_t count, uint32 w, uint32 h, uint32 numBuckets, float* minValue_inout, float* maxValue_inout, float scale)
{
	return ImageHistogram_T(data, count, w, h, numBuckets, minValue_inout, maxValue_inout, scale);
}

uint8* ImageHistogram(const double* data, size_t count, uint32 w, uint32 h, uint32 numBuckets, double* minValue_inout, double* maxValue_inout, float scale)
{
	return ImageHistogram_T(data, count, w, h, numBuckets, minValue_inout, maxValue_inout, scale);
}

// ================================================================================================

#if PLATFORM_PC
void ImageHistogramTest()
{
	const uint32 w = 2048;
	const uint32 h = w/2;
	const uint32 numBuckets = 256;
	const uint32 count = 250*1000*1000;
	const double mean = 0.0;
	const double range = 5.0;
	for (int variance_ = 1; variance_ <= 6; variance_++) {
		const double variance = (double)variance_*0.5;
		printf("\ntesting distribution with variance %f ..\n", variance);
		std::normal_distribution<> d(mean, variance);
		std::mt19937 generator(1);
		std::vector<double> data;
		double sumValue = 0;
		double sumValueSqr = 0;
		for (uint32 i = 0; i < count; i++) {
		#if 1
			const double value = d(generator);
		#else
			// https://en.wikipedia.org/wiki/Normal_distribution#Generating_values_from_normal_distribution
			const double U = (double)(1 + rand())/(double)(RAND_MAX + 1); // (0..1)
			const double V = (double)(1 + rand())/(double)(RAND_MAX + 1); // (0..1)
			const double value = mean + variance*sqrt(-2.0*log(U))*cos(2.0*PI*V); // Box-Muller
		#endif
			sumValue += value;
			sumValueSqr += value*value;
			data.push_back(value);
		}
		const double calculatedMean = sumValue/(double)count;
		const double calculatedVariance = sqrt(sumValueSqr/(double)count);
		printf("mean = %f, variance = %f\n", mean, variance);
		printf("calculated mean = %f\n", calculatedMean);
		printf("calculated variance = %f\n", calculatedVariance);
		double minValue = -range;
		double maxValue = +range;
		const float scale = 0.9f;
		uint8* hist = ImageHistogram(data.data(), data.size(), w, h, numBuckets, &minValue, &maxValue, scale);
		Vec4V* image = new Vec4V[w*h];
		for (uint32 i = 0; i < w*h; i++)
			image[i] = Vec4V(Vec3V((float)hist[i]/255.0f), 1.0f);
		float* graph = new float[w];
		for (uint32 i = 0; i < w; i++) {
			const double x = range*(-1.0 + 2.0*(double)i/(double)(w - 1)); // [-range..range]
			// https://en.wikipedia.org/wiki/Normal_distribution
			double y = exp(-(x - mean)*(x - mean)/(2.0*variance*variance));
			//y /= sqrt(2.0*PI)*variance; // don't divide by sqrt here - we want our graph @x=0 to be 1.0
			graph[i] = (float)y;
		}
		const float thickness = 1.0f;
		const Vec3V color(V_ZAXIS);
		const float opacity = 1.0f;
		ImageGraphOverlay(image, w, h, graph, 0.0f, 1.0f/scale, thickness, color, opacity);
		delete[] graph;
		char path[512];
		sprintf(path, "histogram_test_var_%.4f.png", variance);
		SaveImage(path, image, w, h);
		delete[] hist;
		delete[] image;
	}
}
#endif // PLATFORM_PC

#if defined(_DEBUG)
template <typename T> static float ImageTestGetMaxDiff_T(const T* imageA, const T* imageB, int w, int h)
{
	float maxdiff = 0.0f;
	for (int i = 0; i < w*h; i++)
		maxdiff = Max(Abs(ConvertPixel<float,T>(imageA[i]) - ConvertPixel<float,T>(imageB[i])), maxdiff);
	return maxdiff;
}

template <> float ImageTestGetMaxDiff_T<Vec3f>(const Vec3f* imageA, const Vec3f* imageB, int w, int h)
{
	Vec3f maxdiff(0.0f);
	for (int i = 0; i < w*h; i++)
		maxdiff = Max(Abs(imageA[i] - imageB[i]), maxdiff);
	return MaxElement(maxdiff);
}

template <> float ImageTestGetMaxDiff_T<Vec4V>(const Vec4V* imageA, const Vec4V* imageB, int w, int h)
{
	Vec4V maxdiff(0.0f);
	for (int i = 0; i < w*h; i++)
		maxdiff = Max(Abs(imageA[i] - imageB[i]), maxdiff);
	return MaxElement(maxdiff).f();
}

template <> float ImageTestGetMaxDiff_T<Pixel32>(const Pixel32* imageA, const Pixel32* imageB, int w, int h)
{
	float maxdiff = 0.0f;
	for (int i = 0; i < w*h; i++) {
		maxdiff = Max(Abs(ConvertPixel<float,uint8>(imageA[i].r) - ConvertPixel<float,uint8>(imageB[i].r)), maxdiff);
		maxdiff = Max(Abs(ConvertPixel<float,uint8>(imageA[i].g) - ConvertPixel<float,uint8>(imageB[i].g)), maxdiff);
		maxdiff = Max(Abs(ConvertPixel<float,uint8>(imageA[i].b) - ConvertPixel<float,uint8>(imageB[i].b)), maxdiff);
		maxdiff = Max(Abs(ConvertPixel<float,uint8>(imageA[i].a) - ConvertPixel<float,uint8>(imageB[i].a)), maxdiff);
	}
	return maxdiff;
}

template <> float ImageTestGetMaxDiff_T<Pixel64>(const Pixel64* imageA, const Pixel64* imageB, int w, int h)
{
	float maxdiff = 0.0f;
	for (int i = 0; i < w*h; i++) {
		maxdiff = Max(Abs(ConvertPixel<float,uint16>(imageA[i].r) - ConvertPixel<float,uint16>(imageB[i].r)), maxdiff);
		maxdiff = Max(Abs(ConvertPixel<float,uint16>(imageA[i].g) - ConvertPixel<float,uint16>(imageB[i].g)), maxdiff);
		maxdiff = Max(Abs(ConvertPixel<float,uint16>(imageA[i].b) - ConvertPixel<float,uint16>(imageB[i].b)), maxdiff);
		maxdiff = Max(Abs(ConvertPixel<float,uint16>(imageA[i].a) - ConvertPixel<float,uint16>(imageB[i].a)), maxdiff);
	}
	return maxdiff;
}

void ImageTest()
{
	const char* exts[] = {"dds", "png", "tga", "tif", "bmp", "jpg"};

	// test scalar formats
	{
		const int w = 256;
		const int h = 256;

		uint8* test_uint8 = new uint8[w*h];
		uint16* test_uint16 = new uint16[w*h];
		float* test_float = new float[w*h];

		for (int j = 0; j < h; j++)
		{
			for (int i = 0; i < w; i++)
			{
				const float f = (float)i/(float)(w - 1);

				test_uint8[i + j*w] = ConvertPixel<uint8,float>(f);
				test_uint16[i + j*w] = ConvertPixel<uint16,float>(f);
				test_float[i + j*w] = f;
			}
		}

		for (int i = 0; i < countof(exts); i++)
		{
			char path_uint8[64] = "";
			char path_uint16[64] = "";
			char path_float[64] = "";

			sprintf(path_uint8, "test_uint8.%s", exts[i]);
			sprintf(path_uint16, "test_uint16.%s", exts[i]);
			sprintf(path_float, "test_float.%s", exts[i]);

			SaveImage(path_uint8, test_uint8, w, h, false, true);
			SaveImage(path_uint16, test_uint16, w, h, false, true);
			SaveImage(path_float, test_float, w, h, false, true);

			int w_ = w;
			int h_ = h;
			uint8* test2_uint8 = LoadImage_uint8(path_uint8, w_, h_);
			uint16* test2_uint16 = LoadImage_uint16(path_uint16, w_, h_);
			float* test2_float = LoadImage_float(path_float, w_, h_);

			const float maxdiff_uint8 = ImageTestGetMaxDiff_T<uint8>(test_uint8, test2_uint8, w, h);
			const float maxdiff_uint16 = ImageTestGetMaxDiff_T<uint16>(test_uint16, test2_uint16, w, h);
			const float maxdiff_float = ImageTestGetMaxDiff_T<float>(test_float, test2_float, w, h);

			printf("%f maxdiff %s[uint8]\n", maxdiff_uint8, exts[i]);
			printf("%f maxdiff %s[uint16]\n", maxdiff_uint16, exts[i]);
			printf("%f maxdiff %s[float]\n", maxdiff_float, exts[i]);

			delete[] test2_uint8;
			delete[] test2_uint16;
			delete[] test2_float;
		}

		delete[] test_uint8;
		delete[] test_uint16;
		delete[] test_float;		
	}

	// test vector formats
	{
		const int w = 256;
		const int h = 256;

		Vec3f* test_Vec3f = new Vec3f[w*h];
		Vec4V* test_Vec4V = new Vec4V[w*h];
		Pixel32* test_Pixel32 = new Pixel32[w*h];
		Pixel64* test_Pixel64 = new Pixel64[w*h];

		for (int j = 0; j < h; j++)
		{
			for (int i = 0; i < w; i++)
			{
				const float fi = (float)i/(float)(w - 1);
				const float fj = (float)j/(float)(h - 1);
				const float fk = Max(0.0f, 1.0f - sqrtf((2.0f*fi - 1.0f)*(2.0f*fi - 1.0f) + (2.0f*fj - 1.0f)*(2.0f*fj - 1.0f)));

				test_Vec3f[i + j*w] = Vec3f(fi, fj, fk);
				test_Vec4V[i + j*w] = Vec4V(fi, fj, fk, 1.0f);
				test_Pixel32[i + j*w] = ConvertPixel<Pixel32,Vec3f>(Vec3f(fi, fj, fk));
				test_Pixel64[i + j*w] = ConvertPixel<Pixel64,Vec3f>(Vec3f(fi, fj, fk));
			}
		}

		for (int i = 0; i < countof(exts); i++)
		{
			char path_Vec3f[64] = "";
			char path_Vec4V[64] = "";
			char path_Pixel32[64] = "";
			char path_Pixel64[64] = "";

			sprintf(path_Vec3f, "test_Vec3f.%s", exts[i]);
			sprintf(path_Vec4V, "test_Vec4V.%s", exts[i]);
			sprintf(path_Pixel32, "test_Pixel32.%s", exts[i]);
			sprintf(path_Pixel64, "test_Pixel64.%s", exts[i]);

			SaveImage(path_Vec3f, test_Vec3f, w, h, false, true);
			SaveImage(path_Vec4V, test_Vec4V, w, h, false, true);
			SaveImage(path_Pixel32, test_Pixel32, w, h, false, true);
			SaveImage(path_Pixel64, test_Pixel64, w, h, false, true);

			int w_ = w;
			int h_ = h;
			Vec3f* test2_Vec3f = LoadImage_Vec3f(path_Vec3f, w_, h_);
			Vec4V* test2_Vec4V = LoadImage_Vec4V(path_Vec4V, w_, h_);
			Pixel32* test2_Pixel32 = LoadImage_Pixel32(path_Pixel32, w_, h_);
			Pixel64* test2_Pixel64 = LoadImage_Pixel64(path_Pixel64, w_, h_);

			const float maxdiff_Vec3f = ImageTestGetMaxDiff_T<Vec3f>(test_Vec3f, test2_Vec3f, w, h);
			const float maxdiff_Vec4V = ImageTestGetMaxDiff_T<Vec4V>(test_Vec4V, test2_Vec4V, w, h);
			const float maxdiff_Pixel32 = ImageTestGetMaxDiff_T<Pixel32>(test_Pixel32, test2_Pixel32, w, h);
			const float maxdiff_Pixel64 = ImageTestGetMaxDiff_T<Pixel64>(test_Pixel64, test2_Pixel64, w, h);

			printf("%f maxdiff %s[Vec3f]\n", maxdiff_Vec3f, exts[i]);
			printf("%f maxdiff %s[Vec4V]\n", maxdiff_Vec4V, exts[i]);
			printf("%f maxdiff %s[Pixel32]\n", maxdiff_Pixel32, exts[i]);
			printf("%f maxdiff %s[Pixel64]\n", maxdiff_Pixel64, exts[i]);

			delete[] test2_Vec3f;
			delete[] test2_Vec4V;
			delete[] test2_Pixel32;
			delete[] test2_Pixel64;
		}

		delete[] test_Vec3f;
		delete[] test_Vec4V;
		delete[] test_Pixel32;
		delete[] test_Pixel64;
	}
}
#endif // defined(_DEBUG)
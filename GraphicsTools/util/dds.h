// ============
// common/dds.h
// ============

#ifndef _INCLUDE_COMMON_DDS_H_
#define _INCLUDE_COMMON_DDS_H_

#include "common/common.h"

#define DDS_MAKE_MAGIC_NUMBER(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))
#define DDS_EXPAND_MAGIC_NUMBER(x) ((x) & 255), (((x) >> 8) & 255), (((x) >> 16) & 255), (((x) >> 24) & 255)

// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/directx9_c/directx/graphics/reference/DDSFileReference/ddsfileformat.asp

enum
{
	DDS_DDPF_ALPHAPIXELS     = 0x00000001,
	DDS_DDPF_ALPHA           = 0x00000002,
	DDS_DDPF_FOURCC          = 0x00000004,
	DDS_DDPF_RGB             = 0x00000040,
	DDS_DDPF_RGBA            = DDS_DDPF_RGB | DDS_DDPF_ALPHAPIXELS,
	DDS_DDPF_LUMINANCE       = 0x00020000,
	DDS_DDPF_LUMINANCE_ALPHA = DDS_DDPF_LUMINANCE | DDS_DDPF_ALPHAPIXELS,
};

struct DDS_DDPIXELFORMAT
{
	uint32 dwSize;        // Size of structure. This member must be set to 32.
	uint32 dwFlags;       // Flags to indicate valid fields. Uncompressed formats will usually use DDPF_RGB to indicate an RGB format, while compressed formats will use DDPF_FOURCC with a four-character code.
	uint32 dwFourCC;      // This is the four-character code for compressed formats. dwFlags should include DDPF_FOURCC in this case. For DXTn compression, this is set to "DXT1", "DXT2", "DXT3", "DXT4", or "DXT5".
	uint32 dwRGBBitCount; // For RGB formats, this is the total number of bits in the format. dwFlags should include DDPF_RGB in this case. This value is usually 16, 24, or 32. For A8R8G8B8, this value would be 32.
	uint32 dwRBitMask;
	uint32 dwGBitMask;
	uint32 dwBBitMask;    // For RGB formats, these three fields contain the masks for the red, green, and blue channels. For A8R8G8B8, these values would be 0x00ff0000, 0x0000ff00, and 0x000000ff respectively.
	uint32 dwABitMask;    // For RGB formats, this contains the mask for the alpha channel, if any. dwFlags should include DDPF_ALPHAPIXELS in this case. For A8R8G8B8, this value would be 0xff000000.

	DDS_DDPIXELFORMAT() {}
	DDS_DDPIXELFORMAT(int) { memset(this, 0, sizeof(*this)); }
};

#define DDS_DDPIXELFORMAT9 DDS_DDPIXELFORMAT

enum
{
	DDS_DDSCAPS_ALPHA   = 0x00000002,
	DDS_DDSCAPS_COMPLEX = 0x00000008,
	DDS_DDSCAPS_TEXTURE = 0x00001000,
	DDS_DDSCAPS_MIPMAP  = 0x00400000,
};

enum
{
	DDS_DDSCAPS2_CUBEMAP           = 0x00000200,
	DDS_DDSCAPS2_CUBEMAP_POSITIVEX = 0x00000400,
	DDS_DDSCAPS2_CUBEMAP_NEGATIVEX = 0x00000800,
	DDS_DDSCAPS2_CUBEMAP_POSITIVEY = 0x00001000,
	DDS_DDSCAPS2_CUBEMAP_NEGATIVEY = 0x00002000,
	DDS_DDSCAPS2_CUBEMAP_POSITIVEZ = 0x00004000,
	DDS_DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x00008000,
	DDS_DDSCAPS2_VOLUME            = 0x00200000,
};

#define DDS_DDSCAPS2_CUBEMAP_ALLFACES ( \
	DDS_DDSCAPS2_CUBEMAP_POSITIVEX |\
	DDS_DDSCAPS2_CUBEMAP_NEGATIVEX |\
	DDS_DDSCAPS2_CUBEMAP_POSITIVEY |\
	DDS_DDSCAPS2_CUBEMAP_NEGATIVEY |\
	DDS_DDSCAPS2_CUBEMAP_POSITIVEZ |\
	DDS_DDSCAPS2_CUBEMAP_NEGATIVEZ )

struct DDS_DDCAPS2
{
	uint32 dwCaps1;
	uint32 dwCaps2;
	uint32 Reserved[2];
};

enum
{
	DDS_DDSD_CAPS        = 0x00000001,
	DDS_DDSD_HEIGHT      = 0x00000002,
	DDS_DDSD_WIDTH       = 0x00000004,
	DDS_DDSD_PITCH       = 0x00000008,
	DDS_DDSD_PIXELFORMAT = 0x00001000,
	DDS_DDSD_MIPMAPCOUNT = 0x00020000,
	DDS_DDSD_LINEARSIZE  = 0x00080000,
	DDS_DDSD_DEPTH       = 0x00800000,
};

struct DDS_DDSURFACEDESC2
{
	uint32 dwSize;                     // Size of structure. This member must be set to 124.
	uint32 dwFlags;                    // Flags to indicate valid fields. Always include DDSD_CAPS, DDSD_PIXELFORMAT, DDSD_WIDTH, DDSD_HEIGHT.
	uint32 dwHeight;                   // Height of the main image in pixels
	uint32 dwWidth;                    // Width of the main image in pixels
	uint32 dwPitchOrLinearSize;        // For uncompressed formats, this is the number of bytes per scan line (DWORD> aligned) for the main image. dwFlags should include DDSD_PITCH in this case. For compressed formats, this is the total number of bytes for the main image. dwFlags should be include DDSD_LINEARSIZE in this case.
	uint32 dwDepth;                    // For volume textures, this is the depth of the volume. dwFlags should include DDSD_DEPTH in this case.
	uint32 dwMipMapCount;              // For items with mipmap levels, this is the total number of levels in the mipmap chain of the main image. dwFlags should include DDSD_MIPMAPCOUNT in this case.
	uint32 dwReserved1[11];            // Unused
	DDS_DDPIXELFORMAT ddpfPixelFormat; // 32-byte value that specifies the pixel format structure.
	DDS_DDCAPS2 ddsCaps;               // 16-byte value that specifies the capabilities structure.
	uint32 dwReserved2;                // Unused
};

enum DDS_D3DFORMAT // copied from d3d9types.h 'enum _D3DFORMAT', but prefixed "DDS_" to avoid name conflicts
{
	DDS_D3DFMT_UNKNOWN              = 0,
	DDS_D3DFMT_R8G8B8               = 20,
	DDS_D3DFMT_A8R8G8B8             = 21,
	DDS_D3DFMT_X8R8G8B8             = 22,
	DDS_D3DFMT_R5G6B5               = 23,
	DDS_D3DFMT_X1R5G5B5             = 24,
	DDS_D3DFMT_A1R5G5B5             = 25,
	DDS_D3DFMT_A4R4G4B4             = 26,
	DDS_D3DFMT_R3G3B2               = 27,
	DDS_D3DFMT_A8                   = 28,
	DDS_D3DFMT_A8R3G3B2             = 29,
	DDS_D3DFMT_X4R4G4B4             = 30,
	DDS_D3DFMT_A2B10G10R10          = 31,
	DDS_D3DFMT_A8B8G8R8             = 32,
	DDS_D3DFMT_X8B8G8R8             = 33,
	DDS_D3DFMT_G16R16               = 34,
	DDS_D3DFMT_A2R10G10B10          = 35,
	DDS_D3DFMT_A16B16G16R16         = 36,

	DDS_D3DFMT_A8P8                 = 40,
	DDS_D3DFMT_P8                   = 41,

	DDS_D3DFMT_L8                   = 50,
	DDS_D3DFMT_A8L8                 = 51,
	DDS_D3DFMT_A4L4                 = 52,

	DDS_D3DFMT_V8U8                 = 60,
	DDS_D3DFMT_L6V5U5               = 61,
	DDS_D3DFMT_X8L8V8U8             = 62,
	DDS_D3DFMT_Q8W8V8U8             = 63,
	DDS_D3DFMT_V16U16               = 64,
	DDS_D3DFMT_A2W10V10U10          = 67,

	DDS_D3DFMT_UYVY                 = DDS_MAKE_MAGIC_NUMBER('U', 'Y', 'V', 'Y'),
	DDS_D3DFMT_R8G8_B8G8            = DDS_MAKE_MAGIC_NUMBER('R', 'G', 'B', 'G'),
	DDS_D3DFMT_YUY2                 = DDS_MAKE_MAGIC_NUMBER('Y', 'U', 'Y', '2'),
	DDS_D3DFMT_G8R8_G8B8            = DDS_MAKE_MAGIC_NUMBER('G', 'R', 'G', 'B'),
	DDS_D3DFMT_DXT1                 = DDS_MAKE_MAGIC_NUMBER('D', 'X', 'T', '1'),
	DDS_D3DFMT_DXT2                 = DDS_MAKE_MAGIC_NUMBER('D', 'X', 'T', '2'),
	DDS_D3DFMT_DXT3                 = DDS_MAKE_MAGIC_NUMBER('D', 'X', 'T', '3'),
	DDS_D3DFMT_DXT4                 = DDS_MAKE_MAGIC_NUMBER('D', 'X', 'T', '4'),
	DDS_D3DFMT_DXT5                 = DDS_MAKE_MAGIC_NUMBER('D', 'X', 'T', '5'),

	DDS_D3DFMT_D16_LOCKABLE         = 70,
	DDS_D3DFMT_D32                  = 71,
	DDS_D3DFMT_D15S1                = 73,
	DDS_D3DFMT_D24S8                = 75,
	DDS_D3DFMT_D24X8                = 77,
	DDS_D3DFMT_D24X4S4              = 79,
	DDS_D3DFMT_D16                  = 80,

	DDS_D3DFMT_D32F_LOCKABLE        = 82,
	DDS_D3DFMT_D24FS8               = 83,

	DDS_D3DFMT_D32_LOCKABLE         = 84,
	DDS_D3DFMT_S8_LOCKABLE          = 85,

	DDS_D3DFMT_L16                  = 81,

	DDS_D3DFMT_VERTEXDATA           = 100,
	DDS_D3DFMT_INDEX16              = 101,
	DDS_D3DFMT_INDEX32              = 102,

	DDS_D3DFMT_Q16W16V16U16         = 110,

	DDS_D3DFMT_MULTI2_ARGB8         = DDS_MAKE_MAGIC_NUMBER('M','E','T','1'),

	DDS_D3DFMT_R16F                 = 111,
	DDS_D3DFMT_G16R16F              = 112,
	DDS_D3DFMT_A16B16G16R16F        = 113,

	DDS_D3DFMT_R32F                 = 114,
	DDS_D3DFMT_G32R32F              = 115,
	DDS_D3DFMT_A32B32G32R32F        = 116,

	DDS_D3DFMT_CxV8U8               = 117,

	DDS_D3DFMT_A1                   = 118,

	DDS_D3DFMT_A2B10G10R10_XR_BIAS  = 119,

	DDS_D3DFMT_BINARYBUFFER         = 199,

	DDS_D3DFMT_FORCE_DWORD          = 0x7FFFFFFF
};

struct DDS_HEADER_DXT10
{
	uint32 dxgiFormat; // DDS_DXGI_FORMAT
	uint32 resourceDimension;
	uint32 miscFlag; // see DDS_RESOURCE_MISC_FLAG
	uint32 arraySize;
	uint32 miscFlags2; // see DDS_MISC_FLAGS2
};

enum
{
	// these are used in DDS_HEADER_DXT10 resourceDimension 
	DDS_DIMENSION_TEXTURE1D = 2,
	DDS_DIMENSION_TEXTURE2D = 3,
	DDS_DIMENSION_TEXTURE3D = 4,

	// these are used in DDS_HEADER_DXT10 miscFlag
	DDS_RESOURCE_MISC_TEXTURECUBE = 0x4,
};

enum DDS_DXGI_FORMAT
{
	DDS_DXGI_FORMAT_UNKNOWN                     = 0,
	DDS_DXGI_FORMAT_R32G32B32A32_TYPELESS       = 1,
	DDS_DXGI_FORMAT_R32G32B32A32_FLOAT          = 2,
	DDS_DXGI_FORMAT_R32G32B32A32_UINT           = 3,
	DDS_DXGI_FORMAT_R32G32B32A32_SINT           = 4,
	DDS_DXGI_FORMAT_R32G32B32_TYPELESS          = 5,
	DDS_DXGI_FORMAT_R32G32B32_FLOAT             = 6,
	DDS_DXGI_FORMAT_R32G32B32_UINT              = 7,
	DDS_DXGI_FORMAT_R32G32B32_SINT              = 8,
	DDS_DXGI_FORMAT_R16G16B16A16_TYPELESS       = 9,
	DDS_DXGI_FORMAT_R16G16B16A16_FLOAT          = 10,
	DDS_DXGI_FORMAT_R16G16B16A16_UNORM          = 11,
	DDS_DXGI_FORMAT_R16G16B16A16_UINT           = 12,
	DDS_DXGI_FORMAT_R16G16B16A16_SNORM          = 13,
	DDS_DXGI_FORMAT_R16G16B16A16_SINT           = 14,
	DDS_DXGI_FORMAT_R32G32_TYPELESS             = 15,
	DDS_DXGI_FORMAT_R32G32_FLOAT                = 16,
	DDS_DXGI_FORMAT_R32G32_UINT                 = 17,
	DDS_DXGI_FORMAT_R32G32_SINT                 = 18,
	DDS_DXGI_FORMAT_R32G8X24_TYPELESS           = 19,
	DDS_DXGI_FORMAT_D32_FLOAT_S8X24_UINT        = 20,
	DDS_DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS    = 21,
	DDS_DXGI_FORMAT_X32_TYPELESS_G8X24_UINT     = 22,
	DDS_DXGI_FORMAT_R10G10B10A2_TYPELESS        = 23,
	DDS_DXGI_FORMAT_R10G10B10A2_UNORM           = 24,
	DDS_DXGI_FORMAT_R10G10B10A2_UINT            = 25,
	DDS_DXGI_FORMAT_R11G11B10_FLOAT             = 26,
	DDS_DXGI_FORMAT_R8G8B8A8_TYPELESS           = 27,
	DDS_DXGI_FORMAT_R8G8B8A8_UNORM              = 28,
	DDS_DXGI_FORMAT_R8G8B8A8_UNORM_SRGB         = 29,
	DDS_DXGI_FORMAT_R8G8B8A8_UINT               = 30,
	DDS_DXGI_FORMAT_R8G8B8A8_SNORM              = 31,
	DDS_DXGI_FORMAT_R8G8B8A8_SINT               = 32,
	DDS_DXGI_FORMAT_R16G16_TYPELESS             = 33,
	DDS_DXGI_FORMAT_R16G16_FLOAT                = 34,
	DDS_DXGI_FORMAT_R16G16_UNORM                = 35,
	DDS_DXGI_FORMAT_R16G16_UINT                 = 36,
	DDS_DXGI_FORMAT_R16G16_SNORM                = 37,
	DDS_DXGI_FORMAT_R16G16_SINT                 = 38,
	DDS_DXGI_FORMAT_R32_TYPELESS                = 39,
	DDS_DXGI_FORMAT_D32_FLOAT                   = 40,
	DDS_DXGI_FORMAT_R32_FLOAT                   = 41,
	DDS_DXGI_FORMAT_R32_UINT                    = 42,
	DDS_DXGI_FORMAT_R32_SINT                    = 43,
	DDS_DXGI_FORMAT_R24G8_TYPELESS              = 44,
	DDS_DXGI_FORMAT_D24_UNORM_S8_UINT           = 45,
	DDS_DXGI_FORMAT_R24_UNORM_X8_TYPELESS       = 46,
	DDS_DXGI_FORMAT_X24_TYPELESS_G8_UINT        = 47,
	DDS_DXGI_FORMAT_R8G8_TYPELESS               = 48,
	DDS_DXGI_FORMAT_R8G8_UNORM                  = 49,
	DDS_DXGI_FORMAT_R8G8_UINT                   = 50,
	DDS_DXGI_FORMAT_R8G8_SNORM                  = 51,
	DDS_DXGI_FORMAT_R8G8_SINT                   = 52,
	DDS_DXGI_FORMAT_R16_TYPELESS                = 53,
	DDS_DXGI_FORMAT_R16_FLOAT                   = 54,
	DDS_DXGI_FORMAT_D16_UNORM                   = 55,
	DDS_DXGI_FORMAT_R16_UNORM                   = 56,
	DDS_DXGI_FORMAT_R16_UINT                    = 57,
	DDS_DXGI_FORMAT_R16_SNORM                   = 58,
	DDS_DXGI_FORMAT_R16_SINT                    = 59,
	DDS_DXGI_FORMAT_R8_TYPELESS                 = 60,
	DDS_DXGI_FORMAT_R8_UNORM                    = 61,
	DDS_DXGI_FORMAT_R8_UINT                     = 62,
	DDS_DXGI_FORMAT_R8_SNORM                    = 63,
	DDS_DXGI_FORMAT_R8_SINT                     = 64,
	DDS_DXGI_FORMAT_A8_UNORM                    = 65,
	DDS_DXGI_FORMAT_R1_UNORM                    = 66,
	DDS_DXGI_FORMAT_R9G9B9E5_SHAREDEXP          = 67,
	DDS_DXGI_FORMAT_R8G8_B8G8_UNORM             = 68,
	DDS_DXGI_FORMAT_G8R8_G8B8_UNORM             = 69,
	DDS_DXGI_FORMAT_BC1_TYPELESS                = 70,
	DDS_DXGI_FORMAT_BC1_UNORM                   = 71,
	DDS_DXGI_FORMAT_BC1_UNORM_SRGB              = 72,
	DDS_DXGI_FORMAT_BC2_TYPELESS                = 73,
	DDS_DXGI_FORMAT_BC2_UNORM                   = 74,
	DDS_DXGI_FORMAT_BC2_UNORM_SRGB              = 75,
	DDS_DXGI_FORMAT_BC3_TYPELESS                = 76,
	DDS_DXGI_FORMAT_BC3_UNORM                   = 77,
	DDS_DXGI_FORMAT_BC3_UNORM_SRGB              = 78,
	DDS_DXGI_FORMAT_BC4_TYPELESS                = 79,
	DDS_DXGI_FORMAT_BC4_UNORM                   = 80,
	DDS_DXGI_FORMAT_BC4_SNORM                   = 81,
	DDS_DXGI_FORMAT_BC5_TYPELESS                = 82,
	DDS_DXGI_FORMAT_BC5_UNORM                   = 83,
	DDS_DXGI_FORMAT_BC5_SNORM                   = 84,
	DDS_DXGI_FORMAT_B5G6R5_UNORM                = 85,
	DDS_DXGI_FORMAT_B5G5R5A1_UNORM              = 86,
	DDS_DXGI_FORMAT_B8G8R8A8_UNORM              = 87,
	DDS_DXGI_FORMAT_B8G8R8X8_UNORM              = 88,
	DDS_DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM  = 89,
	DDS_DXGI_FORMAT_B8G8R8A8_TYPELESS           = 90,
	DDS_DXGI_FORMAT_B8G8R8A8_UNORM_SRGB         = 91,
	DDS_DXGI_FORMAT_B8G8R8X8_TYPELESS           = 92,
	DDS_DXGI_FORMAT_B8G8R8X8_UNORM_SRGB         = 93,
	DDS_DXGI_FORMAT_BC6H_TYPELESS               = 94,
	DDS_DXGI_FORMAT_BC6H_UF16                   = 95,
	DDS_DXGI_FORMAT_BC6H_SF16                   = 96,
	DDS_DXGI_FORMAT_BC7_TYPELESS                = 97,
	DDS_DXGI_FORMAT_BC7_UNORM                   = 98,
	DDS_DXGI_FORMAT_BC7_UNORM_SRGB              = 99,
	DDS_DXGI_FORMAT_AYUV                        = 100,
	DDS_DXGI_FORMAT_Y410                        = 101,
	DDS_DXGI_FORMAT_Y416                        = 102,
	DDS_DXGI_FORMAT_NV12                        = 103,
	DDS_DXGI_FORMAT_P010                        = 104,
	DDS_DXGI_FORMAT_P016                        = 105,
	DDS_DXGI_FORMAT_420_OPAQUE                  = 106,
	DDS_DXGI_FORMAT_YUY2                        = 107,
	DDS_DXGI_FORMAT_Y210                        = 108,
	DDS_DXGI_FORMAT_Y216                        = 109,
	DDS_DXGI_FORMAT_NV11                        = 110,
	DDS_DXGI_FORMAT_AI44                        = 111,
	DDS_DXGI_FORMAT_IA44                        = 112,
	DDS_DXGI_FORMAT_P8                          = 113,
	DDS_DXGI_FORMAT_A8P8                        = 114,
	DDS_DXGI_FORMAT_B4G4R4A4_UNORM              = 115,
	DDS_DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT      = 116,
	DDS_DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT      = 117,
	DDS_DXGI_FORMAT_LAST                        = 118,
};

enum DDS_IMAGE_TYPE
{
	DDS_IMAGE_TYPE_UNKNOWN = 1,
	DDS_IMAGE_TYPE_2D,
	DDS_IMAGE_TYPE_3D,
	DDS_IMAGE_TYPE_CUBE,
};

enum DDS_FORMAT_TYPE
{
	DDS_FORMAT_TYPE_UNKNOWN = 0,
	DDS_FORMAT_TYPE_TYPELESS,
	DDS_FORMAT_TYPE_FLOAT,
	DDS_FORMAT_TYPE_UNORM,
	DDS_FORMAT_TYPE_UNORM_SRGB,
	DDS_FORMAT_TYPE_SNORM,
	DDS_FORMAT_TYPE_UINT,
	DDS_FORMAT_TYPE_SINT,
	DDS_FORMAT_TYPE_COMPRESSED,
	DDS_FORMAT_TYPE_OTHER,
};

DDS_D3DFORMAT GetD3DFormatFromDDSPixelFormat(const DDS_DDPIXELFORMAT& ddpf);
const char* GetD3DFormatStr(DDS_D3DFORMAT format, bool shortName = false);
const char* GetDX10FormatStr(DDS_DXGI_FORMAT format, bool shortName = false);
DDS_DXGI_FORMAT GetDX10FormatFromString(const char* str);
bool CompareDDSPixelFormatMask(const DDS_DDPIXELFORMAT& ddpf, uint32 flags, uint32 bitCount, uint32 rMask, uint32 gMask, uint32 bMask, uint32 aMask);
DDS_DXGI_FORMAT GetDX10FormatFromDDSPixelFormat(const DDS_DDPIXELFORMAT& ddpf);
DDS_DDPIXELFORMAT GetDDSPixelFormatFromDX10Format(DDS_DXGI_FORMAT dxgiFormat, bool forceDX9Compatible = false, bool allowUnofficialDX9Formats = true);
DDS_DXGI_FORMAT GeTypelessFormat(DDS_DXGI_FORMAT format);
bool GetDDSInfo(const char* path, int& w, int& h, int* depth, int* mips, DDS_DXGI_FORMAT* dxgiFormat, DDS_IMAGE_TYPE* imageType, int* layerCount, int* offsetToPixelData = NULL);
bool WriteDDSHeader(FILE* dds, DDS_IMAGE_TYPE type, int w, int h, int d, int mips, int layers, const DDS_DDPIXELFORMAT& ddpf, DDS_DXGI_FORMAT dxgiFormat = DDS_DXGI_FORMAT_UNKNOWN, bool bSaveDX10 = false, bool bForceRespectSaveDX10 = false);
int GetDX10FormatBlockSize(DDS_DXGI_FORMAT format);
int GetDX10FormatBitsPerPixel(DDS_DXGI_FORMAT format);
int GetDX10FormatBitsPerComponent(DDS_DXGI_FORMAT format);
bool ConvertPixelsToDX10Format(void* dst, DDS_DXGI_FORMAT format, const void* src_Vec4f, int w, int h, bool sRGB = false);
int GetMaxMipCount(int w, int h, int d, bool bAllowNonPow2Scaling = false);
int GetImageSizeInBytes(DDS_DXGI_FORMAT dxgiFormat, int w, int h, int d = 1, int mips = 1, int layers = 1, DDS_IMAGE_TYPE imageType = DDS_IMAGE_TYPE_2D);
int GetOffsetToSlice(DDS_DXGI_FORMAT dxgiFormat, int w, int h, int d = 1, int mips = 1, int mipIndex = 0, int layerIndex = 0, DDS_IMAGE_TYPE type = DDS_IMAGE_TYPE_2D, int cubeFaceIndex = 0);
bool IsDX10FormatSRGB(DDS_DXGI_FORMAT format);
DDS_FORMAT_TYPE GetDX10FormatType(DDS_DXGI_FORMAT format);

#endif // _INCLUDE_COMMON_DDS_H_
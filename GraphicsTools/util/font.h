// =============
// common/font.h
// =============

#ifndef _INCLUDE_COMMON_FONT_H_
#define _INCLUDE_COMMON_FONT_H_

// =================================================================
// to generate fonts: http://www.angelcode.com/products/bmfont/
// for OpenGL rendering, we want a border pixel around every glyph.
// AngelCode's bitmap font generator doesn't do this by default, you
// need to set Options->Export Options->Padding to 1 for left/up.
// =================================================================

#include "common/common.h"

#include "vmath/vmath_color.h"

class Font
{
public:
	class Glyph
	{
	public:
		Glyph() {}
		Glyph(const char* line);

		int m_id;
		int m_x;
		int m_y;
		int m_w;
		int m_h;
		int m_xoffset;
		int m_yoffset;
		int m_xadvance;
		int m_page; // not used
		int m_chnl; // not used
	};

	static Font* Load(const char* path);
	static Font* LoadConsoleFont(const char* path);

	void AdjustXAdvance(int adj)
	{
		for (int i = 0; i < 256; i++)
			m_glyphs[i].m_xadvance += adj;
	}

	void Release();

	bool SaveImage(const char* path, bool alpha) const;

	void GetStringDimensions(int& sw, int& sh, int& dy, const char* str) const;

	enum
	{
		FONT_DRAW_SHADOW = BIT(0),
		FONT_DRAW_ALPHA = BIT(1),
	};

	int DrawString(PIXELTYPE_uint8  * image, int w, int h, int x, int y, const Pixel32& color, uint32 fontDrawFlags, const char* format, ...) const;
	int DrawString(PIXELTYPE_uint16 * image, int w, int h, int x, int y, const Pixel32& color, uint32 fontDrawFlags, const char* format, ...) const;
	int DrawString(PIXELTYPE_float  * image, int w, int h, int x, int y, const Pixel32& color, uint32 fontDrawFlags, const char* format, ...) const;
	int DrawString(PIXELTYPE_Vec3f  * image, int w, int h, int x, int y, const Pixel32& color, uint32 fontDrawFlags, const char* format, ...) const;
	int DrawString(PIXELTYPE_Vec4V  * image, int w, int h, int x, int y, const Pixel32& color, uint32 fontDrawFlags, const char* format, ...) const;
	int DrawString(PIXELTYPE_Pixel32* image, int w, int h, int x, int y, const Pixel32& color, uint32 fontDrawFlags, const char* format, ...) const;
	int DrawString(PIXELTYPE_Pixel64* image, int w, int h, int x, int y, const Pixel32& color, uint32 fontDrawFlags, const char* format, ...) const;
#if defined(_OPENGL)
	int DrawStringGL(float x, float y, float scale, const Pixel32& color, const char* str) const;
#endif // defined(_OPENGL)

	static void PrintConsoleFontForScreenCapture(FILE* f);

	Glyph    m_glyphs[256];
	Pixel32* m_fontImage;
	int      m_fontImageW;
	int      m_fontImageH;
	int      m_yoffset;
	int      m_maxLineHeight;
#if defined(_OPENGL)
	GLuint   m_texture;
#endif // defined(_OPENGL)
};

#endif // _INCLUDE_COMMON_FONT_H_
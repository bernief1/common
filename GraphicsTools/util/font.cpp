// ===============
// common/font.cpp
// ===============

#include "font.h"

#include "fileutil.h"
#include "imageutil.h"
#include "stringutil.h"

#define DUMP_FONT_INFO (0) // debugging

Font::Glyph::Glyph(const char* line)
{
	char temp[1024] = "";
	strcpy(temp, line);
	ASSERT_ONLY(const char* s =) strtok(temp, " \t");
	DEBUG_ASSERT(strcmp(s, "char") == 0);
	m_id = atoi(strchr(strtok(NULL, " \t"), '=') + 1);
	m_x = atoi(strchr(strtok(NULL, " \t"), '=') + 1);
	m_y = atoi(strchr(strtok(NULL, " \t"), '=') + 1);
	m_w = atoi(strchr(strtok(NULL, " \t"), '=') + 1);
	m_h = atoi(strchr(strtok(NULL, " \t"), '=') + 1);
	m_xoffset = atoi(strchr(strtok(NULL, " \t"), '=') + 1);
	m_yoffset = atoi(strchr(strtok(NULL, " \t"), '=') + 1);
	m_xadvance = atoi(strchr(strtok(NULL, " \t"), '=') + 1);
	m_page = atoi(strchr(strtok(NULL, " \t"), '=') + 1); // TODO -- support multi-page fonts?
	m_chnl = atoi(strchr(strtok(NULL, " \t"), '=') + 1);
}

static void FinalizeFont(Font* font, bool adjustGlyphBounds)
{
	Pixel32* image = font->m_fontImage;
	const int w = font->m_fontImageW;
	const int h = font->m_fontImageH;
	bool hasAlpha = false;
	for (int i = 0; i < w*h; i++) {
		if (image[i].a != 255) {
			hasAlpha = true;
			break;
		}
	}
	if (!hasAlpha) {
		for (int i = 0; i < w*h; i++) {
			Pixel32& c = image[i];
			c.a = c.r;
			c.r = c.g = c.b = 255;
		}
	}
	for (int glyphIndex = 0; glyphIndex < icountof(font->m_glyphs); glyphIndex++) {
		Font::Glyph& glyph = font->m_glyphs[glyphIndex];
		if (glyph.m_xoffset < 0)
			glyph.m_xoffset = 0; // not sure why we'd want this? at least for simple fonts ..
		if (adjustGlyphBounds) {
			int xmin = w;
			int ymin = h;
			int xmax = 0;
			int ymax = 0;
			for (int j = 0; j < glyph.m_h; j++) {
				for (int i = 0; i < glyph.m_w; i++) {
					const int i2 = i + glyph.m_x;
					const int j2 = j + glyph.m_y;
					if (image[i2 + j2*w] != image[w*h - 1]) {
						xmin = Min(i2, xmin);
						ymin = Min(j2, ymin);
						xmax = Max(i2, xmax);
						ymax = Max(j2, ymax);
					}
				}
			}
			if (xmin <= xmax && ymin <= ymax) {
				glyph.m_x = xmin;
				glyph.m_y = ymin;
				glyph.m_w = xmax - xmin + 1;
				glyph.m_h = ymax - ymin + 1;
			}
		}
		font->m_yoffset = Max(-glyph.m_yoffset, font->m_yoffset);
		font->m_maxLineHeight = Max(glyph.m_yoffset + glyph.m_h, font->m_maxLineHeight);
	}
	if (1) // adjust space so it doesn't affect text dimensions vertically (hack)
		font->m_glyphs[' '].m_yoffset = font->m_glyphs['o'].m_yoffset;				
#if defined(_OPENGL)
	GLint prevTexture = 0;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &prevTexture);
	glGenTextures(1, &font->m_texture);
	glBindTexture(GL_TEXTURE_2D, font->m_texture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, font->m_fontImageW, font->m_fontImageH, GL_BGRA_EXT, GL_UNSIGNED_BYTE, font->m_fontImage);
	glBindTexture(GL_TEXTURE_2D, prevTexture);
#endif // defined(_OPENGL)
}

Font* Font::Load(const char* path)
{
	// http://www.angelcode.com/products/bmfont/
	Font* font = NULL;
	const char* ext = strrchr(path, '.');
	bool loadedFromConsoleImage = false;
	if (ext && stricmp(ext, ".fnt") == 0) {
		FILE* f = fopen(path, "r");
		if (f) {
			char imagePath[512] = "";
			strcpy(imagePath, path);
			strcpy(strrchr(imagePath, '.'), ".dds");
			if (!FileExists(imagePath))
				strcpy(strrchr(imagePath, '.'), "_0.tga"); // try the default AngelCode image, which will be "<name>_0.tga"
			int w = 0;
			int h = 0;
			Pixel32* image = LoadImage_Pixel32(imagePath, w, h);
			if (image) {
				font = new Font;
				memset(font, 0, sizeof(*font));
				font->m_fontImage = image;
				font->m_fontImageW = w;
				font->m_fontImageH = h;
				char line[1024];
				fgets(line, sizeof(line), f); // skip first 3 lines
				fgets(line, sizeof(line), f);
				fgets(line, sizeof(line), f);
				fgets(line, sizeof(line), f);
				const int charsCount = atoi(strchr(line, '=') + 1);
				for (int glyphIndex = 0; glyphIndex < charsCount; glyphIndex++) {
					fgets(line, sizeof(line), f);
					const Glyph glyph(line);
					if (glyph.m_id >= 0 && glyph.m_id < icountof(font->m_glyphs))
						font->m_glyphs[glyph.m_id] = glyph;
				}
				FinalizeFont(font, true);
			}
			fclose(f);
		}
	} else if (ext && stricmp(ext, ".png") == 0)
		font = LoadConsoleFont(path);
	if (font) {
	#if DUMP_FONT_INFO
		for (int c = 0x20; c <= 0x7E; c++) {
			printf("glyph '%c': id=%d ('%c'), x=%d, y=%d, w=%d, h=%d, xoff=%d, yoff=%d, xadd=%d\n",
				c,
				font->m_glyphs[c].m_id, font->m_glyphs[c].m_id,
				font->m_glyphs[c].m_x,
				font->m_glyphs[c].m_y,
				font->m_glyphs[c].m_w,
				font->m_glyphs[c].m_h,
				font->m_glyphs[c].m_xoffset,
				font->m_glyphs[c].m_yoffset,
				font->m_glyphs[c].m_xadvance);
		}
	#endif // DUMP_FONT_INFO
	}
	return font;
}

Font* Font::LoadConsoleFont(const char* path)
{
	// expects an image with the font chars starting at 0x20 (' '), 16 columns, 6 rows
	// glyphs are laid out in a grid, background is black
	// the leftmost column of pixels defines the rows - red pixels represent active pixel rows, blue represents the baseline
	// pixel columns are calculated from the glyphs themselves
	// i could make this code robust but i don't want to bother.
	Font* font = nullptr;
	int w = 0;
	int h = 0;
	Pixel32* image = LoadImage_Pixel32(path, w, h);
	if (image) {
		// scan leftmost column of pixels for rows
		class RowInfo {
		public:
			RowInfo(int start) : m_start(start), m_baseline(-1), m_end(-1) {}
			int m_start;
			int m_baseline; // blue pixel
			int m_end;
		};
		std::vector<RowInfo> rows;
		const Pixel32 black(0,0,0);
		const Pixel32 red(255,0,0);
		const Pixel32 blue(0,0,255);
		for (int j = 0; j < h; j++) {
			const Pixel32 c = image[j*w];
			if (c == red) {
				if (j == 0 || image[(j - 1)*w] == black)
					rows.push_back(RowInfo(j));
			} else if (c == blue) {
				ForceAssert(!rows.empty());
				ForceAssert(rows.back().m_baseline == -1);
				rows.back().m_baseline = j;
			} else if (c == black) {
				if (j > 0 && image[(j - 1)*w] != black) {
					ForceAssert(!rows.empty());
					rows.back().m_end = j - 1;
				}
				for (int i = 1; i < w; i++) // check there are no actual glyph pixels
					ForceAssertf(image[i + j*w] == black, "font pixel %d,%d isn't black!", i, j);
			}
		}

		// scan for columns
		class ColInfo {
		public:
			ColInfo(int start) : m_start(start), m_end(-1) {}
			int m_start;
			int m_end;
		};
		std::vector<ColInfo> cols;
		bool prevEmpty = false;
		for (int i = 1; i < w; i++) {
			bool empty = true;
			for (int j = 0; j < h; j++) {
				if (image[i + j*w] != black) {
					empty = false;
					break;
				}
			}
			if (!empty) {
				if (i == 1 || prevEmpty)
					cols.push_back(ColInfo(i));
			} else if (i > 1 && !prevEmpty) {
				ForceAssert(!cols.empty());
				cols.back().m_end = i - 1;
			}
			prevEmpty = empty;
		}
	#if DUMP_FONT_INFO
		for (uint32 i = 0; i < cols.size(); i++) {
			printf("col %d: start=%d, end=%d", i, cols[i].m_start, cols[i].m_end);
			if (i > 0)
				printf(", space=%d", cols[i].m_end - cols[i - 1].m_start - 1);
			printf("\n");
		}
		for (uint32 i = 0; i < rows.size(); i++) {
			printf("row %d: start=%d, base=%d, end=%d", i, rows[i].m_start, rows[i].m_baseline, rows[i].m_end);
			if (i > 0)
				printf(", space=%d", rows[i].m_end - rows[i - 1].m_start - 1);
			printf("\n");
		}
	#endif // DUMP_FONT_INFO
		font = new Font;
		memset(font, 0, sizeof(*font));
		font->m_fontImage = image;
		font->m_fontImageW = w;
		font->m_fontImageH = h;
		int rowIndex = 0;
		int colIndex = 0;
		for (int c = 0x20; c <= 0x7E; c++) {
			Glyph& glyph = font->m_glyphs[c];
			const RowInfo& row = rows[rowIndex];
			const ColInfo& col = cols[colIndex];
			glyph.m_id = c;
			glyph.m_x = col.m_start;
			glyph.m_y = row.m_start;
			glyph.m_w = col.m_end - col.m_start + 1;
			glyph.m_h = row.m_end - row.m_start + 1;
			glyph.m_xoffset = 0;
			glyph.m_yoffset = row.m_baseline - row.m_end;
			glyph.m_xadvance = glyph.m_w + 1;
			glyph.m_page = 0; // not used
			glyph.m_chnl = 0; // not used
			if (++colIndex >= cols.size()) {
				colIndex = 0;
				if (++rowIndex >= rows.size())
					break;
			}
		}
		FinalizeFont(font, false);
	}
	return font;
}

void Font::Release()
{
	if (m_fontImage)
		delete[] m_fontImage;
	memset(this, 0, sizeof(*this));
#if defined(_OPENGL)
	// TODO -- release texture
#endif // defined(_OPENGL)
}

bool Font::SaveImage(const char* path, bool alpha) const
{
	if (m_fontImage) {
		Pixel32* image = m_fontImage;
		if (alpha) {
			image = new Pixel32[m_fontImageW*m_fontImageH];
			for (int i = 0; i < m_fontImageW*m_fontImageH; i++) {
				const uint8 a = m_fontImage[i].a;
				image[i] = Pixel32(a,a,a);
			}
		}
		const bool result = ::SaveImage(path, image, m_fontImageW, m_fontImageH);
		if (image != m_fontImage)
			delete[] image;
		return result;
	} else
		return false;
}

void Font::GetStringDimensions(int& sw, int& sh, int& dy, const char* str) const
{
	const bool regular = true; // set this to false to shrink the bounds around the text in a way that may make the dimensions inconsistent with string length
	sw = 0;
	sh = 0;
	dy = 0;
	for (const char* s = str; *s; s++) {
		const Glyph& glyph = m_glyphs[*s];
		if (regular)
			sw += glyph.m_xadvance;
		else {
			if (s[1]) // next char is not the terminator
				sw += glyph.m_xadvance;
			else
				sw += glyph.m_xoffset + glyph.m_w;
			dy = Max(-glyph.m_yoffset, dy);
			sh = Max(glyph.m_yoffset + glyph.m_h, sh);
		}
	}
	if (regular) {
		sh = m_maxLineHeight;
		dy = m_yoffset;
	}
	sh += dy;
}

template <typename PixelType> static int Font__DrawStringInternal(const Font& font, PixelType* image, int w, int h, int x, int y, const Pixel32& color, uint32 fontDrawFlags, const char* str)
{ 
	if (fontDrawFlags & Font::FONT_DRAW_SHADOW)
		Font__DrawStringInternal(font, image, w, h, x + 2, y + 2, Pixel32(0,0,0,color.a), fontDrawFlags & ~Font::FONT_DRAW_SHADOW, str);
	const Vec4V colorV = ConvertPixel<Vec4V,Pixel32>(color);
	for (const uint8* s = (const uint8*)str; *s; s++) {
		const Font::Glyph& glyph = font.m_glyphs[*s];
		for (int j = 0; j < glyph.m_h; j++) for (int i = 0; i < glyph.m_w; i++) {
			const int ii = x + i + glyph.m_xoffset;
			const int jj = y + j + glyph.m_yoffset;
			if (ii >= 0 && ii < w && jj >= 0 && jj < h) {
				const Vec4V src = ConvertPixel<Vec4V,Pixel32>(font.m_fontImage[(i + glyph.m_x) + (j + glyph.m_y)*font.m_fontImageW])*colorV;
				if (src.wf() > 0.0f) {
					PixelType& dst = image[ii + jj*w];
					Vec4V color = ConvertPixel<Vec4V,PixelType>(dst);
					Vec3V rgb = color.xyz();
					rgb += (src.xyz() - rgb)*src.w();
					color = Vec4V(rgb, color.w());
					if (fontDrawFlags & Font::FONT_DRAW_ALPHA) {
						float alpha = color.wf();
						alpha += (1.0f - alpha)*src.wf();
						color = Vec4V(color.xyz(), alpha);
					}
					dst = ConvertPixel<PixelType,Vec4V>(color);
				}
			}
		}
		x += glyph.m_xadvance;
	}
	return x;
}

int Font::DrawString(PIXELTYPE_uint8  * image, int w, int h, int x, int y, const Pixel32& color, uint32 fontDrawFlags, const char* format, ...) const { GET_STR_VARARGS(str,1024,format); return Font__DrawStringInternal(*this, image, w, h, x, y, color, fontDrawFlags, str); }
int Font::DrawString(PIXELTYPE_uint16 * image, int w, int h, int x, int y, const Pixel32& color, uint32 fontDrawFlags, const char* format, ...) const { GET_STR_VARARGS(str,1024,format); return Font__DrawStringInternal(*this, image, w, h, x, y, color, fontDrawFlags, str); }
int Font::DrawString(PIXELTYPE_float  * image, int w, int h, int x, int y, const Pixel32& color, uint32 fontDrawFlags, const char* format, ...) const { GET_STR_VARARGS(str,1024,format); return Font__DrawStringInternal(*this, image, w, h, x, y, color, fontDrawFlags, str); }
int Font::DrawString(PIXELTYPE_Vec3f  * image, int w, int h, int x, int y, const Pixel32& color, uint32 fontDrawFlags, const char* format, ...) const { GET_STR_VARARGS(str,1024,format); return Font__DrawStringInternal(*this, image, w, h, x, y, color, fontDrawFlags, str); }
int Font::DrawString(PIXELTYPE_Vec4V  * image, int w, int h, int x, int y, const Pixel32& color, uint32 fontDrawFlags, const char* format, ...) const { GET_STR_VARARGS(str,1024,format); return Font__DrawStringInternal(*this, image, w, h, x, y, color, fontDrawFlags, str); }
int Font::DrawString(PIXELTYPE_Pixel32* image, int w, int h, int x, int y, const Pixel32& color, uint32 fontDrawFlags, const char* format, ...) const { GET_STR_VARARGS(str,1024,format); return Font__DrawStringInternal(*this, image, w, h, x, y, color, fontDrawFlags, str); }
int Font::DrawString(PIXELTYPE_Pixel64* image, int w, int h, int x, int y, const Pixel32& color, uint32 fontDrawFlags, const char* format, ...) const { GET_STR_VARARGS(str,1024,format); return Font__DrawStringInternal(*this, image, w, h, x, y, color, fontDrawFlags, str); }

#if defined(_OPENGL)
int Font::DrawStringGL(float x, float y, float scale, const Pixel32& color, const char* str) const
{
	GLint prevTexture = 0;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &prevTexture);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glEnable(GL_TEXTURE_2D);
	glColor4ub(color.r, color.g, color.b, color.a);
	glBegin(GL_QUADS);
	for (const uint8* s = (const uint8*)str; *s; s++) {
		const Glyph& glyph = m_glyphs[*s];
		const float x0 = x + scale*(float)glyph.m_xoffset;
		const float y0 = y + scale*(float)glyph.m_yoffset;
		const float x1 = x0 + scale*(float)glyph.m_w;
		const float y1 = y0 + scale*(float)glyph.m_h;
		const float tx0 = (0.0f + (float)glyph.m_x)/(float)m_fontImageW;
		const float ty0 = (0.0f + (float)glyph.m_y)/(float)m_fontImageH;
		const float tx1 = tx0 + (float)glyph.m_w/(float)m_fontImageW;
		const float ty1 = ty0 + (float)glyph.m_h/(float)m_fontImageH;
		glTexCoord2f(tx0, ty0); glVertex2f(x0, y0);
		glTexCoord2f(tx1, ty0); glVertex2f(x1, y0);
		glTexCoord2f(tx1, ty1); glVertex2f(x1, y1);
		glTexCoord2f(tx0, ty1); glVertex2f(x0, y1);
		x += scale*(float)(glyph.m_xadvance);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, prevTexture);
	return (int)x;
}
#endif // defined(_OPENGL)

void Font::PrintConsoleFontForScreenCapture(FILE* f)
{
	// ===============================
	//   ! " # $ % & ' ( ) * + , - . /
	// 0 1 2 3 4 5 6 7 8 9 : ; < = > ?
	// @ A B C D E F G H I J K L M N O
	// P Q R S T U V W X Y Z [ \ ] ^ _
	// ` a b c d e f g h i j k l m n o
	// p q r s t u v w x y z { | } ~
	// ===============================

	const int firstChar = 0x20;
	const int lastChar = 0x7E;
	const int consoleWidth = 32;
	int x = 0;
	fprintf(f, "\n>>  ");
	for (int c = firstChar; c <= lastChar; c++) {
		if (x >= consoleWidth) {
			fprintf(f, "\n\n>>  ");
			x = 0;
		}
		fprintf(f, " %c", c);
		x += 2;
	}
	fprintf(f, "\n\n");
	system("pause");
}
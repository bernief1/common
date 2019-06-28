// ====================
// common/framebuffer.h
// ====================

#ifndef _INCLUDE_COMMON_FRAMEBUFFER_H_
#define _INCLUDE_COMMON_FRAMEBUFFER_H_

#include "common/common.h"

#if PLATFORM_PC && defined(_CPU_FRAMEBUFFER)

#include "color.h"

class Framebuffer
{
public:
	Framebuffer();
	~Framebuffer();

	void Clear() const;

	void InitSelectID();

	void Resize(unsigned w, unsigned h);

	Pixel32& GetPixelColor_ref(int i, int j) const;
	float& GetPixelDepth_ref(int i, int j) const;
	static uint32 GenSelectID(uint32 selectID);
	uint32 GetPixelSelectID(int i, int j) const;
	//static uint32 GenSelectID24(uint32 selectID); // i've added special 24-bit versions which work with OpenGL framebuffers where I can't seem to read alpha ..
	//uint32 GetPixelSelectID24(int i, int j) const;
	Vec3V_out GetPixelSelectUV(int i, int j) const;

	unsigned m_w;
	unsigned m_h;
	Pixel32* m_color;
	float* m_depth; // linear z
	Pixel32* m_selectID; // buffers for triangle selection - inited on demand
	Pixel32* m_selectUV;
};

template <typename T> inline void FlipVertical(T* buffer, int w, int h)
{
	T* ptr0 = new T[w];
	for (int j = 0; j < h/2; j++) {
		T* ptr1 = buffer + j*w;
		T* ptr2 = buffer + (h - 1 - j)*w;
		memcpy(ptr0, ptr1, w*sizeof(T));
		memcpy(ptr1, ptr2, w*sizeof(T));
		memcpy(ptr2, ptr0, w*sizeof(T));
	}
	delete[] ptr0;
}

#endif // PLATFORM_PC
#endif // _INCLUDE_COMMON_FRAMEBUFFER_H_
// ======================
// common/framebuffer.cpp
// ======================

#include "common/common.h"

#if PLATFORM_PC && defined(_CPU_FRAMEBUFFER)

#include "framebuffer.h"
#include "memory.h"

#include "vmath/vmath_random.h"

Framebuffer::Framebuffer()
{
	memset(this, 0, sizeof(*this));
}

Framebuffer::~Framebuffer()
{
	if (m_color)
		AlignedFree(m_color);
	if (m_depth)
		AlignedFree(m_depth);
	if (m_selectID)
		AlignedFree(m_selectID);
	if (m_selectUV)
		AlignedFree(m_selectUV);
}

void Framebuffer::Clear() const
{
	if (m_color)
		memset(m_color, 0, m_w*m_h*sizeof(Pixel32));
	if (m_depth)
		memset(m_depth, 0, m_w*m_h*sizeof(float));
	if (m_selectID)
		memset(m_selectID, 0, m_w*m_h*sizeof(Pixel32));
	if (m_selectUV)
		memset(m_selectUV, 0, m_w*m_h*sizeof(Pixel32));
}

void Framebuffer::InitSelectID()
{
	if (m_selectID == NULL)
		m_selectID = AlignedAlloc<Pixel32>(m_w*m_h, 32);
	if (m_selectUV == NULL)
		m_selectUV = AlignedAlloc<Pixel32>(m_w*m_h, 32);
}

void Framebuffer::Resize(unsigned w, unsigned h)
{
	if (m_w*m_h < w*h) {
		if (m_color)
			AlignedFree(m_color);
		m_color = AlignedAlloc<Pixel32>(w*h, 32);
		memset(m_color, 0, w*h*sizeof(Pixel32));
		if (m_depth)
			AlignedFree(m_depth);
		m_depth = AlignedAlloc<float>(w*h, 32); // aligned so that the raytracer can vectorize
		memset(m_depth, 0, w*h*sizeof(float));
		if (m_selectID) {
			AlignedFree(m_selectID);
			m_selectID = AlignedAlloc<Pixel32>(w*h, 32);
			memset(m_selectID, 0, w*h*sizeof(Pixel32));
		}
		if (m_selectUV) {
			AlignedFree(m_selectUV);
			m_selectUV = AlignedAlloc<Pixel32>(w*h, 32);
			memset(m_selectUV, 0, w*h*sizeof(Pixel32));
		}
	}
	m_w = w;
	m_h = h;
}

Pixel32& Framebuffer::GetPixelColor_ref(int i, int j) const
{
	i = Clamp(i, 0, (int)m_w - 1);
	j = Clamp(j, 0, (int)m_h - 1);
	return m_color[i + j*m_w];
}

float& Framebuffer::GetPixelDepth_ref(int i, int j) const
{
	i = Clamp(i, 0, (int)m_w - 1);
	j = Clamp(j, 0, (int)m_h - 1);
	return m_depth[i + j*m_w];
}

#define USE_XORSHIFT_FOR_SELECTID (0)

uint32 Framebuffer::GenSelectID(uint32 selectID)
{
#if USE_XORSHIFT_FOR_SELECTID
	if (selectID)
		selectID = XorShift_LRL<uint32,32,13,17,5>(selectID);
#endif // USE_XORSHIFT_FOR_SELECTID
	return selectID;
}

uint32 Framebuffer::GetPixelSelectID(int i, int j) const
{
	if (m_selectID && i >= 0 && j >= 0 && i < (int)m_w && j < (int)m_h) {
		uint32 selectID = m_selectID[i + j*m_w].bgra;
	#if USE_XORSHIFT_FOR_SELECTID
		if (selectID)
			selectID = XorShift_LRL_Inverse<uint32,32,13,17,5>(selectID);
	#endif // USE_XORSHIFT_FOR_SELECTID
		return selectID;
	}
	return 0;
}

//uint32 Framebuffer::GenSelectID24(uint32 selectID)
//{
//#if USE_XORSHIFT_FOR_SELECTID
//	if (selectID)
//		selectID = XorShift_LRL<uint32,24,5,13,17>(selectID);
//#endif // USE_XORSHIFT_FOR_SELECTID
//	return 0xFF000000 | selectID; 
//}
//
//uint32 Framebuffer::GetPixelSelectID24(int i, int j) const
//{
//	if (m_selectID && i >= 0 && j >= 0 && i < (int)m_w && j < (int)m_h) {
//		uint32 selectID = m_selectID[i + j*m_w].bgra & ~0xFF000000;
//	#if USE_XORSHIFT_FOR_SELECTID
//		if (selectID)
//			selectID = XorShift_LRL_Inverse<uint32,24,5,13,17>(selectID);
//	#endif // USE_XORSHIFT_FOR_SELECTID
//		return selectID;
//	}
//	return 0;
//}

Vec3V_out Framebuffer::GetPixelSelectUV(int i, int j) const
{
	if (m_selectUV && i >= 0 && j >= 0 && i < (int)m_w && j < (int)m_h)
		return Vec3V(m_selectUV[i + j*m_w]);
	else
		return Vec3V(-1.0f);
}

#endif // PLATFORM_PC
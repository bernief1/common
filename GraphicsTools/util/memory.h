// ===============
// common/memory.h
// ===============

#ifndef _INCLUDE_COMMON_MEMORY_H_
#define _INCLUDE_COMMON_MEMORY_H_

#include "common/common.h"
#if XXX_GAME
#include "common/MemoryPools.h"
#endif // XXX_GAME

template <typename T> inline T* AlignedAlloc(size_t count, unsigned align)
{
#if PLATFORM_PC
	return reinterpret_cast<T*>(_aligned_malloc(count*sizeof(T), align));
#elif XXX_GAME
	return reinterpret_cast<T*>(MemoryManMemalign(g_currentMemoryPool,align, count*sizeof(T), NoAllocateHigh));
#endif
}

inline void AlignedFree(void* ptr)
{
#if PLATFORM_PC
	_aligned_free(ptr);
#elif XXX_GAME
	MemoryManDeallocate(g_currentMemoryPool, ptr);
#endif
}

#endif // _INCLUDE_COMMON_MEMORY_H_
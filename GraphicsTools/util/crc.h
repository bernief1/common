// ============
// common/crc.h
// ============

#ifndef _INCLUDE_COMMON_CRC_H_
#define _INCLUDE_COMMON_CRC_H_

#include "common/common.h"

uint64 ReverseBits64(uint64 a);
uint64 Crc64(const void* ptr, size_t numBytes, uint64 crc = 0);
template <typename T> inline uint64 Crc64(const T& x, uint64 crc = 0) { return Crc64(&x, sizeof(x), crc); }
uint64 GetFileCrc64(const char* path, uint64 crc = 0, size_t* accumsize = NULL);

#endif // _INCLUDE_COMMON_CRC_H_
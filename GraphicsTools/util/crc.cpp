// ==============
// common/crc.cpp
// ==============

#include "crc.h"

uint64 ReverseBits64(uint64 a)
{
	{ const uint64 mask = 0x5555555555555555ULL; a = ((a >> 0x01) & mask) | ((a & mask) << 0x01); }
	{ const uint64 mask = 0x3333333333333333ULL; a = ((a >> 0x02) & mask) | ((a & mask) << 0x02); }
	{ const uint64 mask = 0x0F0F0F0F0F0F0F0FULL; a = ((a >> 0x04) & mask) | ((a & mask) << 0x04); }
	{ const uint64 mask = 0x00FF00FF00FF00FFULL; a = ((a >> 0x08) & mask) | ((a & mask) << 0x08); }
	{ const uint64 mask = 0x0000FFFF0000FFFFULL; a = ((a >> 0x10) & mask) | ((a & mask) << 0x10); }

	return (a >> 0x20) | (a << 0x20);
}

uint64 Crc64(const void* ptr, size_t numBytes, uint64 crc)
{
	static uint64 table[256] = {0};
	if (table[0] == 0) {
		const uint64 polynomial = ReverseBits64(0x42F0E1EBA9EA3693ULL); // CRC-64-ECMA-182
		for (int i = 0; i < 256; i++) {
			uint64 x = (uint64)i;
			for (int j = 0; j < 8; j++) {
				if (x & 1) { x = (x >> 1) ^ polynomial; }
				else       { x = (x >> 1); }
			}
			table[i] = x;
		}
	}
	const uint8* src = (const uint8*)ptr;
	const uint8* end = &src[numBytes];
	while (src < end)
		crc = table[(uint8)crc ^ *(src++)] ^ (crc >> 8);
	return crc;
}

uint64 GetFileCrc64(const char* path, uint64 crc, size_t* accumsize)
{
	FILE* file = fopen(path, "rb");
	if (file) {
		fseek(file, 0, SEEK_END);
		const size_t size = ftell(file);
		fseek(file, 0, SEEK_SET);
		uint8* data = new uint8[size];
		fread(data, size, 1, file);
		fclose(file);
		crc = Crc64(data, size, crc);
		delete[] data;
		if (accumsize)
			(*accumsize) += size;
	}
	return crc;
}
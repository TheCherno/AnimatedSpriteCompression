#pragma once

#include "Common.h"

class Decompressor
{
private:
	byte* m_Buffer;
	size_t m_Size;
public:
	Decompressor(const String& path);
	Decompressor(byte* buffer, long size);
	~Decompressor();

	byte* Decompress();
	byte* Decompress2();
};

#pragma once

#include <Flinty.h>

struct Animation
{
	int width, height;
	int frames;
	std::vector<int*> data;

	inline ~Animation()
	{
		for (int i = 0; i < data.size(); i++)
			delete[] data[i];
	}
};

class Decompressor
{
private:
	byte* m_Buffer;
	size_t m_Size;
public:
	Decompressor(const String& path);
	Decompressor(byte* buffer, long size);
	~Decompressor();

	Animation* Decompress();
	Animation* Decompress2();
	void Decompress2Benchmark();
};

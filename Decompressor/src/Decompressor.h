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
	void DecompressBenchmark();

	Animation* Decompress1();
	Animation* Decompress2();
	Animation* Decompress3();
	void Decompress2Benchmark();

	Animation* Deserialize3(byte* buffer, long size);
	void Deserialize3Benchmark(byte* buffer, long size);
private:
	byte* DecompressLZ4(const byte* buffer, uint size, uint decompressedSize);
};

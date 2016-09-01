#include "Decompressor.h"
#include <lz4.h>

static char s_StaticBuffer[1024 * 1024 * 10];
static byte* s_DecompressionBuffer = nullptr;

static Animation* Deserialize(byte* buffer, long size);
static Animation* Deserialize2(byte* buffer, long size);

using namespace fl;

inline int GetBPC(byte format)
{
	if (format == 0) // ARGB
		return 4;

	return -1;
}

Decompressor::Decompressor(const String& path)
{
	m_Buffer = FileSystem::ReadFile(path, &m_Size);
}

Decompressor::Decompressor(byte* buffer, long size)
	: m_Buffer(buffer), m_Size(size)
{
}

Decompressor::~Decompressor()
{
}

Animation* Decompressor::Decompress()
{
	FL_ASSERT(m_Buffer[0] == 0x08 && m_Buffer[1] == 0x08);
	if (m_Buffer[2] == 0x0 && m_Buffer[3] == 0x2)
		return Deserialize3(m_Buffer, m_Size);

	FL_ASSERT(false);
	return nullptr;
}

void Decompressor::DecompressBenchmark()
{
	FL_ASSERT(m_Buffer[0] == 0x08 && m_Buffer[1] == 0x08);
	FL_ASSERT(m_Buffer[2] == 0x0 && m_Buffer[3] == 0x2);
	Deserialize3Benchmark(m_Buffer, m_Size);
}

Animation* Decompressor::Decompress1()
{
	return Deserialize(m_Buffer, m_Size);
}

Animation* Decompressor::Decompress2()
{
	return Deserialize2(m_Buffer, m_Size);
}

void Decompressor::Decompress2Benchmark()
{
	byte* buffer = m_Buffer;
	size_t size = m_Size;

	struct Header
	{
		byte h0, h1;
		byte quality;
		byte format;
		byte r0, r1;
		ushort frames;
		ushort width, height;
	};

	const Header& header = *(Header*)buffer;
	buffer += sizeof(Header);

	int bpc = GetBPC(header.format);
	int frame = 0;
	while (buffer)
	{
		byte frameType = *(byte*)buffer++;
		byte compressionType = *(byte*)buffer++;

		int* pixels = (int*)s_StaticBuffer;
		if (frameType == 0) // Keyframe
		{
			// Assume no compression for now
			memcpy(pixels, buffer, header.width * header.height * bpc);
			buffer += header.width * header.height * bpc;
		}
		else // Delta frame
		{
			int* currentPixel = pixels;
			int* end = pixels + header.width * header.height;
			while (currentPixel < end)
			{
				ushort skipcount = *(ushort*)&buffer[0];
				ushort copycount = *(ushort*)&buffer[2];
				buffer += 4;
				memcpy(currentPixel + skipcount, buffer, copycount * bpc);
				buffer += copycount * bpc;
				currentPixel += skipcount + copycount;
			}
		}

		++frame;
		if (frame >= header.frames)
			break;
	}
}

Animation* Deserialize(byte* buffer, long size)
{
	struct Header
	{
		byte h0, h1;
		byte quality;
		byte format;
		byte r0, r1;
		ushort frames;
		ushort width, height;
	};

	const Header& header = *(Header*)buffer;
	buffer += sizeof(Header);
	int frame = 0;

	int bpc = GetBPC(header.format);
	while (buffer)
	{
		byte frameType = *(byte*)buffer++;
		byte compressionType = *(byte*)buffer++;

		if (frameType == 0) // Keyframe
		{
			// Assume no compression for now
			int* pixels = (int*)s_StaticBuffer;
			memcpy(pixels, buffer, header.width * header.height * bpc);
			buffer += header.width * header.height * bpc;
		}
		else // Delta frame
		{
			int* pixels = (int*)s_StaticBuffer;
			int* currentPixel = pixels;
			int* end = pixels + header.width * header.height;
			while (currentPixel < end)
			{
				ushort nextStep = *(ushort*)buffer;
				buffer += 2;
				int count = nextStep & 0x7FFF;
				if (nextStep & 0x8000)
				{
					memcpy(currentPixel, buffer, count * bpc);
					currentPixel += count;
					buffer += count * bpc;
				}
				else
				{
					currentPixel += count;
				}
			}
		}

		++frame;
		if (frame >= header.frames)
			break;
	}
	return nullptr;
}

Animation* Deserialize2(byte* buffer, long size)
{
	struct Header
	{
		byte h0, h1;
		byte quality;
		byte format;
		byte r0, r1;
		ushort frames;
		ushort width, height;
	};

	const Header& header = *(Header*)buffer;
	buffer += sizeof(Header);

	Animation* result = new Animation();
	result->frames = header.frames;
	result->width = header.width;
	result->height = header.height;
	result->data.resize(result->frames);

	int bpc = GetBPC(header.format);
	int frame = 0;
	while (buffer)
	{
		result->data[frame] = new int[header.width * header.height];

		byte frameType = *(byte*)buffer++;
		byte compressionType = *(byte*)buffer++;

		int* pixels = (int*)s_StaticBuffer;
		if (frameType == 0) // Keyframe
		{
			// Assume no compression for now
			memcpy(pixels, buffer, header.width * header.height * bpc);
			buffer += header.width * header.height * bpc;
		}
		else // Delta frame
		{
			int* currentPixel = pixels;
			int* end = pixels + header.width * header.height;
			while (currentPixel < end)
			{
				ushort skipcount = *(ushort*)&buffer[0];
				ushort copycount = *(ushort*)&buffer[2];
				buffer += 4;
				memcpy(currentPixel + skipcount, buffer, copycount * bpc);
				buffer += copycount * bpc;
				currentPixel += skipcount + copycount;
			}
		}
		memcpy(result->data[frame], pixels, header.width * header.height * bpc);

		++frame;
		if (frame >= header.frames)
			break;
	}
	return result;
}

Animation* Decompressor::Deserialize3(byte* buffer, long fileSize)
{
	struct Header
	{
		byte h0, h1;			// 08 08
		byte v0, v1;			// Major/Minor version
		byte quality;			// 0-5
		byte compression;		// 0 = none, 1 = LZ4
		byte format;			// 0 = ARGB
		byte r0;				// Reserved
		ushort frames;			// number of frames
		ushort width, height;	// Size of animation
		uint decompressionBound;
	};

	const byte* start = buffer;
	const Header& header = *(Header*)buffer;
	buffer += sizeof(Header);

	FL_LOG("Header information:");
	FL_LOG("\tQuality=%d", header.quality);
	FL_LOG("\tCompression=%d", header.compression);
	FL_LOG("\tFormat=%d", header.format);
	FL_LOG("\tFrames=%d", header.frames);
	FL_LOG("\tSize=%d,%d", header.width, header.height);

	Animation* result = new Animation();
	result->frames = header.frames;
	result->width = header.width;
	result->height = header.height;
	result->data.resize(result->frames);

	s_DecompressionBuffer = new byte[header.decompressionBound];

	int bpp = GetBPC(header.format);
	int frame = 0;
	while (buffer)
	{
		result->data[frame] = new int[header.width * header.height];

		byte frameType = *(byte*)buffer++;
		uint size = *(uint*)buffer;
		buffer += 4;

		int* pixels = (int*)s_StaticBuffer;
		if (frameType == 0) // Keyframe
		{
			if (header.compression == 1) // LZ4
			{
				uint decompressedSize = *(uint*)buffer;
				buffer += 4;
				FL_ASSERT((buffer + size) - start <= fileSize);
				byte* data = DecompressLZ4(buffer, size, decompressedSize);
				memcpy(pixels, data, decompressedSize);
				buffer += size;
			}
			else
			{
				FL_ASSERT((header.width * header.height * bpp) == size);
				memcpy(pixels, buffer, header.width * header.height * bpp);
				buffer += size;
			}
		}
		else if (frameType == 1) // Delta frame
		{
			if (header.compression == 1) // LZ4
			{
				uint decompressedSize = *(uint*)buffer;
				buffer += 4;
				FL_ASSERT((buffer + size) - start <= fileSize);
				byte* data = DecompressLZ4(buffer, size, decompressedSize);
				const byte* dataStart = data;
				const byte* dataEnd = dataStart + decompressedSize;
				buffer += size;

				int* currentPixel = pixels;
				int* end = pixels + header.width * header.height;
				while (currentPixel < end)
				{
					ushort skipcount = *(ushort*)&data[0];
					ushort copycount = *(ushort*)&data[2];
					data += 4;
					FL_ASSERT(copycount * bpp <= dataEnd - data);
					memcpy(currentPixel + skipcount, data, copycount * bpp);
					data += copycount * bpp;
					currentPixel += skipcount + copycount;
				}
			}
			else
			{
				int* currentPixel = pixels;
				int* end = pixels + header.width * header.height;
				while (currentPixel < end)
				{
					ushort skipcount = *(ushort*)&buffer[0];
					ushort copycount = *(ushort*)&buffer[2];
					buffer += 4;
					memcpy(currentPixel + skipcount, buffer, copycount * bpp);
					buffer += copycount * bpp;
					currentPixel += skipcount + copycount;
				}
			}
		}
		memcpy(result->data[frame], pixels, header.width * header.height * bpp);

		frame++;
		if (frame >= header.frames)
			break;
	}
	return result;
}


void Decompressor::Deserialize3Benchmark(byte* buffer, long fileSize)
{
	struct Header
	{
		byte h0, h1;			// 08 08
		byte v0, v1;			// Major/Minor version
		byte quality;			// 0-5
		byte compression;		// 0 = none, 1 = LZ4
		byte format;			// 0 = ARGB
		byte r0;				// Reserved
		ushort frames;			// number of frames
		ushort width, height;	// Size of animation
		ushort padding;
		uint decompressionBound;
	};

	const byte* start = buffer;
	const Header& header = *(Header*)buffer;
	buffer += sizeof(Header);

	s_DecompressionBuffer = new byte[header.decompressionBound];

	int bpp = GetBPC(header.format);
	int frame = 0;
	while (buffer)
	{
		byte frameType = *(byte*)buffer++;
		uint size = *(uint*)buffer;
		buffer += 4;

		int* pixels = (int*)s_StaticBuffer;
		if (frameType == 0) // Keyframe
		{
			if (header.compression == 1) // LZ4
			{
				uint decompressedSize = *(uint*)buffer;
				buffer += 4;
				FL_ASSERT((buffer + size) - start <= fileSize);
				byte* data = DecompressLZ4(buffer, size, decompressedSize);
				memcpy(pixels, data, decompressedSize);
				buffer += size;
			}
			else
			{
				FL_ASSERT((header.width * header.height * bpp) == size);
				memcpy(pixels, buffer, header.width * header.height * bpp);
				buffer += size;
			}
		}
		else if (frameType == 1) // Delta frame
		{
			if (header.compression == 1) // LZ4
			{
				uint decompressedSize = *(uint*)buffer;
				buffer += 4;
				FL_ASSERT((buffer + size) - start <= fileSize);
				byte* data = DecompressLZ4(buffer, size, decompressedSize);
				const byte* dataStart = data;
				const byte* dataEnd = dataStart + decompressedSize;
				buffer += size;

				int* currentPixel = pixels;
				int* end = pixels + header.width * header.height;
				while (currentPixel < end)
				{
					ushort skipcount = *(ushort*)&data[0];
					ushort copycount = *(ushort*)&data[2];
					data += 4;
					FL_ASSERT(copycount * bpp <= dataEnd - data);
					memcpy(currentPixel + skipcount, data, copycount * bpp);
					data += copycount * bpp;
					currentPixel += skipcount + copycount;
				}
			}
			else
			{
				int* currentPixel = pixels;
				int* end = pixels + header.width * header.height;
				while (currentPixel < end)
				{
					ushort skipcount = *(ushort*)&buffer[0];
					ushort copycount = *(ushort*)&buffer[2];
					buffer += 4;
					memcpy(currentPixel + skipcount, buffer, copycount * bpp);
					buffer += copycount * bpp;
					currentPixel += skipcount + copycount;
				}
			}
		}
		frame++;
		if (frame >= header.frames)
			break;
	}
}

byte* Decompressor::DecompressLZ4(const byte* buffer, uint size, uint decompressedSize)
{
	int bytes = LZ4_decompress_fast((const char*)buffer, (char*)s_DecompressionBuffer, decompressedSize);
	FL_ASSERT(bytes == size);
	return s_DecompressionBuffer;
}
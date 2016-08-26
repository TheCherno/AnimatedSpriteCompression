#include "Decompressor.h"

static char s_StaticBuffer[1024 * 1024 * 10];

Animation* Deserialize(byte* buffer, long size);
Animation* Deserialize2(byte* buffer, long size);

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
#include "Decompressor.h"

static char s_StaticBuffer[1024 * 1024 * 10];

void Deserialize(byte* buffer, long size);
void Deserialize2(byte* buffer, long size);

inline int GetBPC(byte format)
{
	if (format == 0) // ARGB
		return 4;

	return -1;
}

Decompressor::Decompressor(const String& path)
{
	m_Buffer = ReadFile(path.c_str(), &m_Size);
}

Decompressor::Decompressor(byte* buffer, long size)
	: m_Buffer(buffer), m_Size(size)
{
}

Decompressor::~Decompressor()
{
}

byte* Decompressor::Decompress()
{
	Deserialize(m_Buffer, m_Size);
	return nullptr;
}

byte* Decompressor::Decompress2()
{
	Deserialize2(m_Buffer, m_Size);
	return nullptr;
}

void Deserialize(byte* buffer, long size)
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
}

void Deserialize2(byte* buffer, long size)
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

	int bpc = GetBPC(header.format);
	int frame = 0;
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
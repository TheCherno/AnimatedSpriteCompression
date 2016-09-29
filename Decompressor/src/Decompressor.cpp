#include "Decompressor.h"
#include <lz4.h>
#include <zstd.h>

#include "CompressionFormat.h"

static const int s_StaticBufferSize = 1024 * 1024 * 10;
static char s_StaticBuffer[s_StaticBufferSize];
static byte* s_DecompressionBuffer = nullptr;
static int s_DecompressionBufferSize = 0;

static Animation* Deserialize(byte* buffer, long size);
static Animation* Deserialize2(byte* buffer, long size);

using namespace fl;

enum CompressionType
{
	NONE = 0,
	LZ4 = 1,
	ZSTD = 2
};

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
	else if (m_Buffer[2] == 0x0 && m_Buffer[3] == 0x4)
		return Deserialize4(m_Buffer, m_Size);

	FL_ASSERT(false);
	return nullptr;
}

void Decompressor::DecompressBenchmark()
{
	FL_ASSERT(m_Buffer[0] == 0x08 && m_Buffer[1] == 0x08);
	if (m_Buffer[2] == 0x0 && m_Buffer[3] == 0x2)
		Deserialize3Benchmark(m_Buffer, m_Size);
	else if (m_Buffer[2] == 0x0 && m_Buffer[3] == 0x3)
		Deserialize4Benchmark(m_Buffer, m_Size);
}

Animation* Decompressor::Decompress1()
{
	return Deserialize(m_Buffer, m_Size);
}

Animation* Decompressor::Decompress2()
{
	return Deserialize2(m_Buffer, m_Size);
}

Animation* Decompressor::Decompress3()
{
	return Deserialize3(m_Buffer, m_Size);
}

Animation* Decompressor::Decompress4()
{
	return Deserialize4(m_Buffer, m_Size);
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

	s_DecompressionBufferSize = header.decompressionBound;
	s_DecompressionBuffer = new byte[s_DecompressionBufferSize];

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
			if (header.compression > 0)
			{
				uint decompressedSize = *(uint*)buffer;
				buffer += 4;
				FL_ASSERT((buffer + size) - start <= fileSize);
				byte* data = nullptr;
				if (header.compression == LZ4)
					data = DecompressLZ4(buffer, size, decompressedSize);
				else if (header.compression == ZSTD)
					data = DecompressZSTD(buffer, size, decompressedSize);
				FL_ASSERT(data);
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
			if (header.compression > 0)
			{
				uint decompressedSize = *(uint*)buffer;
				buffer += 4;
				FL_ASSERT((buffer + size) - start <= fileSize);
				byte* data = nullptr;
				if (header.compression == LZ4)
					data = DecompressLZ4(buffer, size, decompressedSize);
				else if (header.compression == ZSTD)
					data = DecompressZSTD(buffer, size, decompressedSize);
				FL_ASSERT(data);
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

	s_DecompressionBufferSize = header.decompressionBound;
	s_DecompressionBuffer = new byte[s_DecompressionBufferSize];

	int bpp = GetBPC(header.format);
	int frame = 0;

	if (header.compression > 0)
	{
		while (buffer)
		{
			byte frameType = *(byte*)buffer++;
			uint size = *(uint*)buffer;
			buffer += 4;

			int* pixels = (int*)s_StaticBuffer;
			if (frameType == 0) // Keyframe
			{
				uint decompressedSize = *(uint*)buffer;
				buffer += 4;
				FL_ASSERT((buffer + size) - start <= fileSize);
				byte* data = nullptr;
				if (header.compression == LZ4)
					data = DecompressLZ4(buffer, size, decompressedSize);
				else if (header.compression == ZSTD)
					data = DecompressZSTD(buffer, size, decompressedSize);
				memcpy(pixels, data, decompressedSize);
				buffer += size;
			}
			else if (frameType == 1) // Delta frame
			{
				uint decompressedSize = *(uint*)buffer;
				buffer += 4;
				FL_ASSERT((buffer + size) - start <= fileSize);
				byte* data = nullptr;
				if (header.compression == LZ4)
					data = DecompressLZ4(buffer, size, decompressedSize);
				else if (header.compression == ZSTD)
					data = DecompressZSTD(buffer, size, decompressedSize);
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
			frame++;
			if (frame >= header.frames)
				break;
		}
	}
	else
	{
		while (buffer)
		{
			byte frameType = *(byte*)buffer++;
			uint size = *(uint*)buffer;
			buffer += 4;

			int* pixels = (int*)s_StaticBuffer;
			if (frameType == 0) // Keyframe
			{
				FL_ASSERT((header.width * header.height * bpp) == size);
				memcpy(pixels, buffer, header.width * header.height * bpp);
				buffer += size;
			}
			else if (frameType == 1) // Delta frame
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
			frame++;
			if (frame >= header.frames)
				break;
		}
	}
}


Animation* Decompressor::Deserialize4(byte* buffer, long fileSize)
{
	const byte* start = buffer;
	Header header;
	Metadata metadata;
	memcpy(&header, buffer, 20);
	buffer += 20;
	
	// Event table
	header.eventCount = *(ushort*)buffer;
	buffer += 2;
	header.events = new Header::Event[header.eventCount];
	for (ushort i = 0; i < header.eventCount; i++)
	{
		header.events[i].eventNameLength = *(ushort*)buffer;
		buffer += 2;
		header.events[i].eventName = new char[header.events[i].eventNameLength + 1];
		header.events[i].eventName[header.events[i].eventNameLength] = 0;
		memcpy(header.events[i].eventName, buffer, header.events[i].eventNameLength);
		buffer += header.events[i].eventNameLength;
		header.events[i].startFrame = *(ushort*)buffer;
		buffer += 2;
		header.events[i].endFrame = *(ushort*)buffer;
		buffer += 2;

		Metadata::Event e;
		e.name = String(header.events[i].eventName);
		e.startFrame = header.events[i].startFrame;
		e.endFrame = header.events[i].endFrame;
		metadata.events.push_back(e);
	}

	// Animation table
	header.animationCount = *(ushort*)buffer;
	buffer += 2;
	header.animations = new Header::Animation[header.animationCount];
	for (ushort i = 0; i < header.animationCount; i++)
	{
		header.animations[i].animationNameLength = *(ushort*)buffer;
		buffer += 2;
		header.animations[i].animationName = new char[header.animations[i].animationNameLength + 1];
		header.animations[i].animationName[header.animations[i].animationNameLength] = 0;
		memcpy(header.animations[i].animationName, buffer, header.animations[i].animationNameLength);
		buffer += header.animations[i].animationNameLength;
		header.animations[i].startFrameOffset = *(uint*)buffer;
		buffer += 4;
		header.animations[i].startFrameIndex = *(ushort*)buffer;
		buffer += 2;
		header.animations[i].endFrameIndex = *(ushort*)buffer;
		buffer += 2;
		header.animations[i].mode = *(Header::AnimationMode*)buffer;
		buffer++;

		Metadata::Animation a;
		a.name = String(header.animations[i].animationName);
		a.startFrame = header.animations[i].startFrameIndex;
		a.endFrame = header.animations[i].endFrameIndex;
		metadata.animations.push_back(a);
	}

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

	s_DecompressionBufferSize = header.decompressionBound;
	s_DecompressionBuffer = new byte[s_DecompressionBufferSize];

	int bpp = GetBPC((byte)header.format);
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
			if ((byte)header.compression > 0)
			{
				uint decompressedSize = *(uint*)buffer;
				buffer += 4;
				FL_ASSERT((buffer + size) - start <= fileSize);
				byte* data = nullptr;
				if (header.compression == Header::CompressionType::LZ4)
					data = DecompressLZ4(buffer, size, decompressedSize);
				else if (header.compression == Header::CompressionType::ZSTD)
					data = DecompressZSTD(buffer, size, decompressedSize);
				FL_ASSERT(data);
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
			if ((byte)header.compression > 0)
			{
				uint decompressedSize = *(uint*)buffer;
				buffer += 4;
				FL_ASSERT((buffer + size) - start <= fileSize);
				byte* data = nullptr;
				if (header.compression == Header::CompressionType::LZ4)
					data = DecompressLZ4(buffer, size, decompressedSize);
				else if (header.compression == Header::CompressionType::ZSTD)
					data = DecompressZSTD(buffer, size, decompressedSize);
				FL_ASSERT(data);
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


void Decompressor::Deserialize4Benchmark(byte* buffer, long fileSize)
{
	const byte* start = buffer;
	Header header;
	memcpy(&header, buffer, 20);
	buffer += 20;

	// Event table
	header.eventCount = *(ushort*)buffer;
	buffer += 2;
	header.events = new Header::Event[header.eventCount];
	for (ushort i = 0; i < header.eventCount; i++)
	{
		header.events[i].eventNameLength = *(ushort*)buffer;
		buffer += 2;
		header.events[i].eventName = new char[header.events[i].eventNameLength + 1];
		header.events[i].eventName[header.events[i].eventNameLength] = 0;
		memcpy(header.events[i].eventName, buffer, header.events[i].eventNameLength);
		buffer += header.events[i].eventNameLength;
		header.events[i].startFrame = *(ushort*)buffer;
		buffer += 2;
		header.events[i].endFrame = *(ushort*)buffer;
		buffer += 2;
	}

	// Animation table
	header.animationCount = *(ushort*)buffer;
	buffer += 2;
	header.animations = new Header::Animation[header.animationCount];
	for (ushort i = 0; i < header.eventCount; i++)
	{
		header.animations[i].animationNameLength = *(ushort*)buffer;
		buffer += 2;
		header.animations[i].animationName = new char[header.animations[i].animationNameLength + 1];
		header.animations[i].animationName[header.animations[i].animationNameLength] = 0;
		memcpy(header.animations[i].animationName, buffer, header.animations[i].animationNameLength);
		buffer += header.animations[i].animationNameLength;
		header.animations[i].startFrameOffset = *(uint*)buffer;
		buffer += 4;
		header.animations[i].startFrameIndex = *(ushort*)buffer;
		buffer += 2;
		header.animations[i].endFrameIndex = *(ushort*)buffer;
		buffer += 2;
		header.animations[i].mode = *(Header::AnimationMode*)buffer;
		buffer++;
	}


	s_DecompressionBufferSize = header.decompressionBound;
	s_DecompressionBuffer = new byte[s_DecompressionBufferSize];

	int bpp = GetBPC((byte)header.format);
	int frame = 0;

	if ((byte)header.compression > 0)
	{
		while (buffer)
		{
			byte frameType = *(byte*)buffer++;
			uint size = *(uint*)buffer;
			buffer += 4;

			int* pixels = (int*)s_StaticBuffer;
			if (frameType == 0) // Keyframe
			{
				uint decompressedSize = *(uint*)buffer;
				buffer += 4;
				FL_ASSERT((buffer + size) - start <= fileSize);
				byte* data = nullptr;
				if (header.compression == Header::CompressionType::LZ4)
					data = DecompressLZ4(buffer, size, decompressedSize);
				else if (header.compression == Header::CompressionType::ZSTD)
					data = DecompressZSTD(buffer, size, decompressedSize);
				memcpy(pixels, data, decompressedSize);
				buffer += size;
			}
			else if (frameType == 1) // Delta frame
			{
				uint decompressedSize = *(uint*)buffer;
				buffer += 4;
				FL_ASSERT((buffer + size) - start <= fileSize);
				byte* data = nullptr;
				if (header.compression == Header::CompressionType::LZ4)
					data = DecompressLZ4(buffer, size, decompressedSize);
				else if (header.compression == Header::CompressionType::ZSTD)
					data = DecompressZSTD(buffer, size, decompressedSize);
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
			frame++;
			if (frame >= header.frames)
				break;
		}
	}
	else
	{
		while (buffer)
		{
			byte frameType = *(byte*)buffer++;
			uint size = *(uint*)buffer;
			buffer += 4;

			int* pixels = (int*)s_StaticBuffer;
			if (frameType == 0) // Keyframe
			{
				FL_ASSERT((header.width * header.height * bpp) == size);
				memcpy(pixels, buffer, header.width * header.height * bpp);
				buffer += size;
			}
			else if (frameType == 1) // Delta frame
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
			frame++;
			if (frame >= header.frames)
				break;
		}
	}
}

byte* Decompressor::DecompressLZ4(const byte* buffer, uint size, uint decompressedSize)
{
	int bytes = LZ4_decompress_fast((const char*)buffer, (char*)s_DecompressionBuffer, decompressedSize);
	FL_ASSERT(bytes == size);
	return s_DecompressionBuffer;
}

byte* Decompressor::DecompressZSTD(const byte* buffer, uint size, uint decompressedSize)
{
	size_t bytes = ZSTD_decompress(s_DecompressionBuffer, decompressedSize, buffer, size);
	FL_ASSERT(!ZSTD_isError(bytes));
#ifdef FL_DEBUG
	if (ZSTD_isError(bytes))
	{
		FL_LOG("ZSTD Decompression error: %s", ZSTD_getErrorName(bytes));
	}
#endif
	return s_DecompressionBuffer;
}
#include "Compressor.h"

#include <algorithm>
#include <lz4.h>

struct Header
{
	byte h0, h1;			// 08 08
	byte quality;			// 0-5
	byte format;			// 0 = ARGB
	byte r0, r1;			// Reserved
	ushort frames;			// number of frames
	ushort width, height;	// Size of animation
};

Compressor::Compressor(const std::vector<Sprite>& sprites, byte quality, int windowSize)
	: m_Sprites(sprites), m_Quality(quality), m_WindowSize(windowSize)
{
	m_SimilarityThreshold = 10 * quality;
	m_CompressionMode = 1;
}

Compressor::~Compressor()
{
}

void Compressor::Compress(const String& toFile)
{
	BufferStream stream;

	WriteHeader(stream);
	WriteKeyframe(stream, m_Sprites[0]);
	for (int i = 1; i < m_Sprites.size(); i++)
	{
		WriteDeltaFrame(stream, m_Sprites[i], (int*)m_Sprites[i - 1].pixels);
		std::cout << "Compressed frame " << i << " - " << 0 << " bytes" << std::endl;
	}

	FILE* file = fopen(toFile.c_str(), "wb");
	FL_ASSERT(file);
	fwrite(stream.GetBuffer(), stream.GetSize(), 1, file);
}

void Compressor::WriteHeader(BufferStream& stream)
{
	Header header;
	header.h0 = 0x08;
	header.h1 = 0x08;
	header.quality = m_Quality;
	header.format = 0;
	header.r0 = header.r1 = 0;
	header.frames = (ushort)m_Sprites.size();
	header.width = (ushort)m_Sprites[0].width;
	header.height = (ushort)m_Sprites[0].height;

	stream.Write(&header);
}

void Compressor::WriteKeyframe(BufferStream& stream, const Sprite& sprite)
{
	byte type = 0; // keyframe
	stream.Write(&type);
	stream.Write(&m_CompressionMode);

	byte* pixels = sprite.pixels;
	uint size = sprite.width * sprite.height * 4; // RGBA - 4 bpp
	if (m_CompressionMode > 0)
		pixels = ApplyEntropyCompression(pixels, size, &size);

	stream.Write(pixels, size);
}

void Compressor::WriteDeltaFrame(BufferStream& stream, const Sprite& frame, int* previousBuffer)
{
	byte type = 1; // delta frame
	stream.Write(&type);
	stream.Write(&m_CompressionMode);

	int pixelCount = frame.width * frame.height;
	int skipped = 0;
	int copied = 0;
	bool emit = false;

	BufferStream packetStream;
	int* currentBuffer = (int*)frame.pixels;
	for (int i = 0; i < pixelCount; i++)
	{
		int pp = previousBuffer[i];
		int cp = currentBuffer[i];

		if (copied == 0xffff || skipped == 0xffff)
		{
			EmitPacket(packetStream, currentBuffer, i, (ushort)skipped, (ushort)copied);
			emit = false;
			skipped = 0;
			copied = 0;
		}

		if (pp == cp || ChannelDiff(pp, cp) < m_SimilarityThreshold)
		{
			if (emit)
			{
				EmitPacket(packetStream, currentBuffer, i, (ushort)skipped, (ushort)copied);
				emit = false;
				skipped = 0;
				copied = 0;
			}
			skipped++;
		}
		else
		{
			copied++;
			if (skipped < m_WindowSize)
			{
				copied += skipped;
				skipped = 0;
			}
			else
			{
				emit = true;
			}
		}
	}

	if (skipped > 0 || copied > 0)
		EmitPacket(packetStream, currentBuffer, pixelCount, (ushort)skipped, (ushort)copied);

	const byte* buffer = packetStream.GetBuffer();
	uint bufferSize = packetStream.GetSize();
	if (m_CompressionMode > 0)
		buffer = ApplyEntropyCompression(buffer, bufferSize, &bufferSize);

	stream.Write(buffer, bufferSize);
}

void Compressor::EmitPacket(BufferStream& stream, int* pixels, int cursor, ushort skipped, ushort copied)
{
	stream.Write(&skipped);
	stream.Write(&copied);
	stream.Write(pixels + cursor - copied, copied);
}

byte* Compressor::ApplyEntropyCompression(const byte* buffer, uint size, uint* outSize)
{
	int maxCompressedSize = LZ4_compressBound(size);
	byte* dst = new byte[maxCompressedSize];
	int bytes = LZ4_compress_default((const char*)buffer, (char*)dst, size, maxCompressedSize);
	if (outSize)
		*outSize = bytes;
	byte* result = new byte[bytes];
	memcpy(result, dst, bytes);
	delete[] dst;
	return result;
}

int Compressor::ChannelDiff(int c0, int c1)
{
	byte* a = (byte*)&c0;
	byte* b = (byte*)&c1;

	int result = 0;
	for (int i = 0; i < 4; i++)
		result = std::max(result, std::abs(a[i] - b[i]));
	return result;
}

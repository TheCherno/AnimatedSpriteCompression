#include "Compressor.h"

#include <algorithm>
#include <lz4.h>
#include <zstd.h>

static byte s_Version[2] = { 0x0, 0x4 }; // TODO: INCREMENT when making breaking changes!

Compressor::Compressor(const std::vector<Sprite>& sprites, const Metadata& metadata, byte quality, Header::CompressionType compressionType, int windowSize)
	: m_Sprites(sprites), m_Metadata(metadata), m_Quality(quality), m_CompressionMode(compressionType), m_WindowSize(windowSize), m_DecompressionBound(0)
{
	m_SimilarityThreshold = 10 * quality;
}

Compressor::~Compressor()
{
}

// TODO: Create animation start frame table to avoid this search
bool Compressor::IsAnimation(ushort index)
{
	for (int i = 0; i < m_Metadata.animations.size(); i++)
	{
		if (m_Metadata.animations[i].startFrame == index)
			return true;
	}
	return false;
}

void Compressor::Compress(const String& toFile)
{
	BufferStream stream;
	WriteHeader(stream);
	WriteKeyframe(stream, m_Sprites[0], 0);
	uint deltaStartPos = stream.GetSize();
	for (int i = 1; i < m_Sprites.size(); i++)
	{
		if (IsAnimation(i))
			WriteKeyframe(stream, m_Sprites[i], i);
		else
			WriteDeltaFrame(stream, m_Sprites[i], (int*)m_Sprites[i - 1].pixels);
		std::cout << "Compressed frame " << i << " - " << 0 << " bytes" << std::endl;
	}
	stream.Write(&m_DecompressionBound, 1, offsetof(Header, decompressionBound));

	if (m_CompressionMode == (Header::CompressionType)20) // Compress all delta frames together
	{
		const byte* deltaStart = stream.GetBuffer() + deltaStartPos;
		const byte* end = stream.GetBuffer() + stream.GetSize();

		uint size;
		byte* data = ApplyEntropyCompression(deltaStart, end - deltaStart, &size);
		BufferStream fileStream;
		fileStream.Write(stream.GetBuffer(), deltaStart - stream.GetBuffer());
		fileStream.Write(data, size);
		uint bound = std::max(m_DecompressionBound, size);
		fileStream.Write(&bound, 1, offsetof(Header, decompressionBound));

		FILE* file = fopen(toFile.c_str(), "wb");
		FL_ASSERT(file);
		fwrite(fileStream.GetBuffer(), fileStream.GetSize(), 1, file);
		FL_LOG("Wrote %d bytes", fileStream.GetSize());
		fclose(file);
	}
	else
	{
		FILE* file = fopen(toFile.c_str(), "wb");
		FL_ASSERT(file);
		fwrite(stream.GetBuffer(), stream.GetSize(), 1, file);
		FL_LOG("Wrote %d bytes", stream.GetSize());
		fclose(file);
	}
}

void Compressor::WriteHeader(BufferStream& stream)
{
	Header header;
	header.h0 = 0x08;
	header.h1 = 0x08;
	header.v0 = s_Version[0];
	header.v1 = s_Version[1];
	header.quality = m_Quality;
	header.compression = m_CompressionMode;
	header.format = Header::Format::ARGB;
	header.r0 = 0;
	header.frames = (ushort)m_Sprites.size();
	header.width = (ushort)m_Sprites[0].width;
	header.height = (ushort)m_Sprites[0].height;
	header.padding = 0;
	header.decompressionBound = 0; // To be set later

	stream.WriteBytes((const byte*)&header, 20);

	// Event Table
	const std::vector<Metadata::Event>& events = m_Metadata.events;
	header.eventCount = events.size();
	header.events = new Header::Event[header.eventCount];
	for (ushort i = 0; i < header.eventCount; i++)
	{
		header.events[i].eventNameLength = events[i].name.size();
		header.events[i].eventName = (char*)events[i].name.c_str();
		header.events[i].startFrame = events[i].startFrame;
		header.events[i].endFrame = events[i].endFrame;
	}

	stream.Write(&header.eventCount);
	for (ushort i = 0; i < header.eventCount; i++)
	{
		stream.Write(&header.events[i].eventNameLength);
		stream.Write(header.events[i].eventName, header.events[i].eventNameLength);
		stream.Write(&header.events[i].startFrame);
		stream.Write(&header.events[i].endFrame);
	}

	// Animation Table
	const std::vector<Metadata::Animation>& animations = m_Metadata.animations;
	header.animationCount = animations.size();
	header.animations = new Header::Animation[header.animationCount];
	for (ushort i = 0; i < header.animationCount; i++)
	{
		header.animations[i].animationNameLength = animations[i].name.size();
		header.animations[i].animationName = (char*)animations[i].name.c_str();
		header.animations[i].startFrameIndex = animations[i].startFrame;
		header.animations[i].startFrameOffset = 0x00; // set later
		header.animations[i].endFrameIndex = animations[i].endFrame;
		header.animations[i].mode = animations[i].mode;
	}

	stream.Write(&header.animationCount);
	for (ushort i = 0; i < header.animationCount; i++)
	{
		stream.Write(&header.animations[i].animationNameLength);
		stream.Write(header.animations[i].animationName, header.animations[i].animationNameLength);
		m_AnimationOffsets[header.animations[i].startFrameIndex] = stream.GetSize();
		stream.Write(&header.animations[i].startFrameOffset);
		stream.Write(&header.animations[i].startFrameIndex);
		stream.Write(&header.animations[i].endFrameIndex);
		stream.Write(&header.animations[i].mode);
	}
}

void Compressor::WriteKeyframe(BufferStream& stream, const Sprite& sprite, ushort index)
{
	if (m_AnimationOffsets.find(index) != m_AnimationOffsets.end())
	{
		uint offset = stream.GetSize();
		stream.Write(&offset, 1, m_AnimationOffsets[index]);
	}

	byte type = 0; // keyframe
	stream.Write(&type);

	byte* pixels = sprite.pixels;
	uint rawSize = sprite.width * sprite.height * 4; // RGBA - 4 bpp
	uint bufferSize = rawSize;
	if ((byte)m_CompressionMode > 0)
	{
		uint compressedSize;
		pixels = ApplyEntropyCompression(pixels, rawSize, &compressedSize);
		stream.Write(&compressedSize);
		bufferSize = compressedSize;
	}
	
	stream.Write(&rawSize);
	stream.Write(pixels, bufferSize);

	m_DecompressionBound = std::max(m_DecompressionBound, rawSize);
}

void Compressor::WriteDeltaFrame(BufferStream& stream, const Sprite& frame, int* previousBuffer)
{
	byte type = 1; // delta frame
	stream.Write(&type);

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
			emit = true;
		}
	}

	if (skipped > 0 || copied > 0)
		EmitPacket(packetStream, currentBuffer, pixelCount, (ushort)skipped, (ushort)copied);

	const byte* buffer = packetStream.GetBuffer();
	uint rawSize = packetStream.GetSize();
	uint bufferSize = rawSize;
	if ((byte)m_CompressionMode > 0)
	{
		uint compressedSize;
		buffer = ApplyEntropyCompression(buffer, rawSize, &compressedSize);
		stream.Write(&compressedSize);
		bufferSize = compressedSize;
	}
	stream.Write(&rawSize);
	stream.Write(buffer, bufferSize);
	m_DecompressionBound = std::max(m_DecompressionBound, rawSize);
	FL_LOG("Wrote delta frame: rs=%d, cs=%d, ratio=%.2f", rawSize, bufferSize, rawSize / (float)bufferSize);
}

void Compressor::EmitPacket(BufferStream& stream, int* pixels, int cursor, ushort skipped, ushort copied)
{
	stream.Write(&skipped);
	stream.Write(&copied);
	stream.Write(pixels + cursor - copied, copied);
}

byte* Compressor::ApplyEntropyCompression(const byte* buffer, uint size, uint* outSize)
{
	byte* dst = nullptr;
	int bytes = 0;
	if (m_CompressionMode == Header::CompressionType::LZ4)
	{
		int maxCompressedSize = LZ4_compressBound(size);
		dst = new byte[maxCompressedSize];
		bytes = LZ4_compress_default((const char*)buffer, (char*)dst, size, maxCompressedSize);
	}
	else if (m_CompressionMode == Header::CompressionType::ZSTD)
	{
		int maxCompressedSize = ZSTD_compressBound(size);
		dst = new byte[maxCompressedSize];
		bytes = ZSTD_compress(dst, maxCompressedSize, buffer, size, 1);
	}

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

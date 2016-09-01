#pragma once

#include <Flinty.h>
#include "BufferStream.h"
#include "Sprite.h"

class Compressor
{
private:
	int m_Quality;
	int m_SimilarityThreshold;
	byte m_CompressionMode;
	std::vector<Sprite> m_Sprites;
	int m_WindowSize;
	uint m_DecompressionBound;
public:
	Compressor(const std::vector<Sprite>& sprites, byte quality, int windowSize = 16);
	~Compressor();

	void Compress(const String& toFile);
private:
	void WriteHeader(BufferStream& stream);
	void WriteKeyframe(BufferStream& stream, const Sprite& sprite);
	void WriteDeltaFrame(BufferStream& stream, const Sprite& frame, int* previousBuffer);

	int ChannelDiff(int c0, int c1);

	void EmitPacket(BufferStream& stream, int* pixels, int cursor, ushort skipped, ushort copied);
	byte* ApplyEntropyCompression(const byte* buffer, uint size, uint* outSize = nullptr);
};

#pragma once

#include <Flinty.h>
#include "BufferStream.h"
#include "Sprite.h"

#include "CompressionFormat.h"

class Compressor
{
private:
	int m_Quality;
	int m_SimilarityThreshold;
	Header::CompressionType m_CompressionMode;
	std::vector<Sprite> m_Sprites;
	Metadata m_Metadata;
	int m_WindowSize;
	uint m_DecompressionBound;
	std::unordered_map<ushort, uint> m_AnimationOffsets;
public:
	Compressor(const std::vector<Sprite>& sprites, const Metadata& metadata, byte quality, Header::CompressionType compressionType, int windowSize);
	~Compressor();

	void Compress(const String& toFile);
private:
	void WriteHeader(BufferStream& stream);
	void WriteKeyframe(BufferStream& stream, const Sprite& sprite, ushort index);
	void WriteDeltaFrame(BufferStream& stream, const Sprite& frame, int* previousBuffer);

	int ChannelDiff(int c0, int c1);

	void EmitPacket(BufferStream& stream, int* pixels, int cursor, ushort skipped, ushort copied);
	byte* ApplyEntropyCompression(const byte* buffer, uint size, uint* outSize = nullptr);

	bool IsAnimation(ushort index);
};

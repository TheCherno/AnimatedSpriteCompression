#pragma once

#include "fl/String.h"

typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned int uint;

struct Header
{
	enum class CompressionType : byte
	{
		NONE = 0, LZ4 = 1, ZSTD = 2
	};

	enum class Format : byte
	{
		ARGB = 0
	};

	enum class AnimationMode : byte
	{
		NONE = 0, NORMAL = 1, LOOP = 2, PINGPONG = 3
	};

	byte h0, h1;					// [ 0] 08 08
	byte v0, v1;					// [ 2] Major/Minor version
	byte quality;					// [ 4] 0-5
	CompressionType compression;	// [ 5]
	Format format;					// [ 6]
	byte r0;						// [ 7] Reserved
	ushort frames;					// [ 8] number of frames
	ushort width, height;			// [10] Size of animation
	ushort padding;					// [14]
	uint decompressionBound;		// [16] Size of largest decompression stream

	struct Event
	{
		ushort eventNameLength;
		char* eventName;
		ushort startFrame, endFrame;
	};

	ushort eventCount;
	Event* events;

	struct Animation
	{
		ushort animationNameLength;
		char* animationName;
		uint startFrameOffset;
		ushort startFrameIndex;
		ushort endFrameIndex;
		AnimationMode mode;
	};

	ushort animationCount;
	Animation* animations;
};

struct Metadata
{
	struct Event
	{
		String name;
		ushort startFrame, endFrame;
	};

	struct Animation
	{
		String name;
		ushort startFrame, endFrame;
		Header::AnimationMode mode;
	};

	std::vector<Event> events;
	std::vector<Animation> animations;
};
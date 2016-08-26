#pragma once

#include <Flinty.h>

struct Sprite
{
	uint width, height;
	byte* pixels;

	Sprite(const String& path);
};

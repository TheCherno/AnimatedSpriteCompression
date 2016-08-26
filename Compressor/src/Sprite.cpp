#include "Sprite.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Sprite::Sprite(const String& path)
{
	int c;
	int w, h;
	pixels = stbi_load(path.c_str(), &w, &h, &c, STBI_rgb_alpha);
	width = (uint)w;
	height = (uint)h;
}
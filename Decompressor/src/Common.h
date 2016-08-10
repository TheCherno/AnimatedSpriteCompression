#pragma once

#include <iostream>
#include <string>
#include <vector>

typedef std::string String;

typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned int uint;

inline byte* ReadFile(const char* path, long* outSize = nullptr)
{
	FILE* file = fopen(path, "rb");
	if (!file)
		return nullptr;

	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	fseek(file, 0, SEEK_SET);

	if (outSize)
		*outSize = size;

	byte* buffer = new byte[size];
	fread(buffer, 1, size, file);
	fclose(file);
	return buffer;
}

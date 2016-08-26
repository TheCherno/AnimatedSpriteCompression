#pragma once

#include "Common.h"
#include "Decompressor.h"

struct DecompressionResult
{
	float time;
	size_t size;
	Animation* animation;
};

class DecompressionTest
{
private:
	String m_AssetsDirectory;
public:
	DecompressionTest(const String& assetsDirectory = "");

	std::vector<DecompressionResult> RunAllTests();
	std::vector<DecompressionResult> RunWindowTests();
private:
	float GetMin(float* array, int size);
};
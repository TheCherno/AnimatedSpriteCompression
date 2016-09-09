#pragma once

#include "Common.h"
#include "Decompressor.h"

struct DecompressionResult
{
	String id;
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
	std::vector<DecompressionResult> RunWindowTests(const String& compression);
	std::vector<DecompressionResult> RunLZ4Tests();
	std::vector<DecompressionResult> RunUncompressedTests();
private:
	float GetMin(float* array, int size);
};
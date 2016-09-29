#include "DecompressionTest.h"

using namespace fl;

static const int s_IterationCount = 10;

DecompressionTest::DecompressionTest(const String& assetsDirectory)
	: m_AssetsDirectory(assetsDirectory)
{
	if (assetsDirectory.size() && assetsDirectory[assetsDirectory.size() - 1] != '/')
		m_AssetsDirectory += '/';
}

std::vector<DecompressionResult> DecompressionTest::RunAllTests()
{
	// std::vector<DecompressionResult> uncompressedWindowResults256 = RunWindowTests("256", "uncompressed");
	// std::vector<DecompressionResult> lz4WindowResults256 = RunWindowTests("256", "lz4");
	std::vector<DecompressionResult> uncompressedWindowResults512 = RunWindowTests("512", "uncompressed");
	std::vector<DecompressionResult> lz4WindowResults512 = RunWindowTests("512", "lz4");
	std::vector<DecompressionResult> zstdWindowResults512 = RunWindowTests("512", "zstd");

	std::vector<DecompressionResult> results;
	// results.insert(results.end(), uncompressedWindowResults256.begin(), uncompressedWindowResults256.end());
	// results.insert(results.end(), lz4WindowResults256.begin(), lz4WindowResults256.end());
	results.insert(results.end(), uncompressedWindowResults512.begin(), uncompressedWindowResults512.end());
	results.insert(results.end(), lz4WindowResults512.begin(), lz4WindowResults512.end());
	results.insert(results.end(), zstdWindowResults512.begin(), zstdWindowResults512.end());
	return results;
}

std::vector<DecompressionResult> DecompressionTest::RunWindowTests(const String& size, const String& compression)
{
	const int count = 7;
	const String files[count] = {
		"ws0", "ws8", "ws16", "ws24", "ws32","ws40", "ws48"
	};

	float timeBuffer[s_IterationCount];

	std::vector<DecompressionResult> results(count);
	for (int i = 0; i < count; i++)
	{
		String path = m_AssetsDirectory + "WS/" + size + "/butterfly" + size + "-" + compression + "-" + files[i] + ".bin";
		byte* buffer = fl::FileSystem::ReadFile(path, &results[i].size);
		FL_ASSERT(buffer);
		Decompressor decompressor(buffer, results[i].size);

		for (int i = 0; i < s_IterationCount; i++)
		{
			Timer timer;
			decompressor.DecompressBenchmark();
			timeBuffer[i] = timer.ElapsedMillis();
		}
		results[i].time = GetMin(timeBuffer, s_IterationCount);
		results[i].id = path;
	}

	return results;
}

std::vector<DecompressionResult> DecompressionTest::RunUncompressedTests()
{
	std::vector<DecompressionResult> results(1);
	String path = m_AssetsDirectory + "WS/512/butterfly512-uncompressed-ws16.bin";
	byte* buffer = fl::FileSystem::ReadFile(path, &results[0].size);
	FL_ASSERT(buffer);
	Decompressor decompressor(buffer, results[0].size);
	float timeBuffer[s_IterationCount];
	for (int i = 0; i < s_IterationCount; i++)
	{
		Timer timer;
		decompressor.DecompressBenchmark();
		timeBuffer[i] = timer.ElapsedMillis();
	}
	results[0].time = GetMin(timeBuffer, s_IterationCount);
	results[0].id = path;
	return results;
}

std::vector<DecompressionResult> DecompressionTest::RunLZ4Tests()
{
	std::vector<DecompressionResult> results(1);
	String path = m_AssetsDirectory + "WS/512/butterfly512-lz4-ws16.bin";
	byte* buffer = fl::FileSystem::ReadFile(path, &results[0].size);
	FL_ASSERT(buffer);
	Decompressor decompressor(buffer, results[0].size);
	float timeBuffer[s_IterationCount];
	for (int i = 0; i < s_IterationCount; i++)
	{
		Timer timer;
		decompressor.DecompressBenchmark();
		timeBuffer[i] = timer.ElapsedMillis();
	}
	results[0].time = GetMin(timeBuffer, s_IterationCount);
	results[0].id = path;
	return results;
}

float DecompressionTest::GetMin(float* array, int size)
{
	float min = FLT_MAX;
	for (int i = 0; i < size; i++)
	{
		if (array[i] < min)
			min = array[i];
	}
	return min;
}
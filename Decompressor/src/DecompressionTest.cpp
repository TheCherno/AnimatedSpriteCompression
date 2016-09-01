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
	std::vector<DecompressionResult> uncompressedResults = RunUncompressedTests();
	std::vector<DecompressionResult> lz4Results = RunLZ4Tests();
	// std::vector<DecompressionResult> windowResults = RunWindowTests();

	std::vector<DecompressionResult> results;
	results.insert(results.end(), uncompressedResults.begin(), uncompressedResults.end());
	results.insert(results.end(), lz4Results.begin(), lz4Results.end());
	// results.insert(results.end(), windowResults.begin(), windowResults.end());
	return results;
}

std::vector<DecompressionResult> DecompressionTest::RunWindowTests()
{
	const int count = 14;
	const String files[count] = {
		"ws0", "ws4", "ws8", "ws12",
		"ws16", "ws20", "ws24", "ws28",
		"ws32", "ws36", "ws40", "ws44",
		"ws48", "ws52"
	};

	float timeBuffer[s_IterationCount];

	std::vector<DecompressionResult> results(count);
	for (int i = 0; i < count; i++)
	{
		String path = m_AssetsDirectory + "WS/animation-" + files[i] + ".bin";
		byte* buffer = fl::FileSystem::ReadFile(path, &results[i].size);
		FL_ASSERT(buffer);
		Decompressor decompressor(buffer, results[i].size);

		// Decompress once to obtain data
		results[i].animation = decompressor.Decompress2();

		for (int i = 0; i < s_IterationCount; i++)
		{
			Timer timer;
			decompressor.Decompress2Benchmark();
			timeBuffer[i] = timer.ElapsedMillis();
		}
		results[i].time = GetMin(timeBuffer, s_IterationCount);
	}

	return results;
}

std::vector<DecompressionResult> DecompressionTest::RunUncompressedTests()
{
	std::vector<DecompressionResult> results(1);
	String path = m_AssetsDirectory + "animation-uncompressed.bin";
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
	return results;
}

std::vector<DecompressionResult> DecompressionTest::RunLZ4Tests()
{
	std::vector<DecompressionResult> results(1);
	String path = m_AssetsDirectory + "animation-lz4.bin";
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
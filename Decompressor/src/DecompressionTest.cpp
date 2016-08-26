#include "DecompressionTest.h"

using namespace fl;

static const int s_IterationCount = 1000;

DecompressionTest::DecompressionTest(const String& assetsDirectory)
	: m_AssetsDirectory(assetsDirectory)
{
	if (assetsDirectory.size() && assetsDirectory[assetsDirectory.size() - 1] != '/')
		m_AssetsDirectory += '/';
}

std::vector<DecompressionResult> DecompressionTest::RunAllTests()
{
	return RunWindowTests();
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
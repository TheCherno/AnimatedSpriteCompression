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

#include <chrono>

class Timer
{
private:
	typedef std::chrono::high_resolution_clock HighResolutionClock;
	typedef std::chrono::duration<float, std::milli> milliseconds_type;

	std::chrono::time_point<HighResolutionClock> m_Start;
public:
	inline Timer()
	{
		Reset();
	}

	inline void Reset()
	{
		m_Start = HighResolutionClock::now();
	}

	inline float Elapsed()
	{
		return std::chrono::duration_cast<milliseconds_type>(HighResolutionClock::now() - m_Start).count() / 1000.0f;
	}

	inline float ElapsedMillis()
	{
		return Elapsed() * 1000.0f;
	}
};
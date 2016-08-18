#pragma once

#include <iostream>
#include <string>
#include <vector>

#if defined(PLATFORM_ANDROID)
// Android specific includes
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#endif

typedef std::string String;

typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned int uint;

#if defined(PLATFORM_WIN32)
	#define LOG(string, ...) printf(string##"\n", __VA_ARGS__)
#elif defined(PLATFORM_ANDROID)
	#define LOG(...) ((void)__android_log_print(ANDROID_LOG_WARN, "DecompressionTest", __VA_ARGS__))
#endif

#ifdef DEBUG
	#if defined(PLATFORM_WIN32)
		#define ASSERT(x) if (!(x)) { __debugbreak(); }
	#elif defined(PLATFORM_ANDROID)
		#define ASSERT(x) if (!(x)) { __asm__ volatile(".inst 0xd4200000"); }
	#endif
#else
	#define ASSERT(x)
#endif

#if defined(PLATFORM_WIN32)

void Init();

#elif defined(PLATFORM_ANDROID)

void Init(JNIEnv* env, jobject mainView);

#endif

byte* ReadFile(const String& path, size_t* outSize = nullptr);

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
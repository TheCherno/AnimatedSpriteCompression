#pragma once

#include <stddef.h>

#include "fl/Types.h"

// TODO: Move to precompiled header
#include <iostream>

// Common data structures
#include <vector>
#include <unordered_map>
#include <unordered_set>

#ifdef FL_PLATFORM_ANDROID
#include <android/log.h>
#endif

// Some typedefs to rename C++'s questionable names
template<typename T> using HashSet = std::unordered_set<T>;
template<typename K, typename V> using HashMap = std::unordered_map<K, V>;

#ifdef FL_PLATFORM_WINDOWS
	#ifdef FL_CORE_DLL
		#define FL_API __declspec(dllexport)
	#else
		#define FL_API __declspec(dllimport)
	#endif
#else
	#define FL_API
#endif

namespace fl {
	extern void GLClearError();
	extern bool GLLogCall(const char* function, const char* file, int line);
}

#ifdef FL_DEBUG
	#if defined(FL_PLATFORM_WINDOWS)
		#define FL_ASSERT(x) \
			if (!(x)) {\
				std::cout << "*************************" << std::endl; \
				std::cout << "    ASSERTION FAILED!    " << std::endl; \
				std::cout << "*************************" << std::endl; \
				std::cout << __FILE__ << ": " << __LINE__ << std::endl; \
				std::cout << "Condition: " << #x << std::endl; \
				__debugbreak(); \
			}
	#elif defined(FL_PLATFORM_ANDROID)
		#define FL_ASSERT(x) if (!(x)) { __asm__ volatile(".inst 0xd4200000"); }
	#endif
#else
	#define FL_ASSERT(x, ...)
#endif


#ifdef FL_DEBUG
	#define GLCall(x) fl::GLClearError();\
		x; \
		if (!fl::GLLogCall(#x, __FILE__, __LINE__)) FL_ASSERT(false);
#else
	#define GLCall(x) x
#endif

#if defined(FL_PLATFORM_WINDOWS)
	#define FL_LOG(string, ...) printf(string##"\n", __VA_ARGS__)
#elif defined(FL_PLATFORM_ANDROID)
	#define FL_LOG(...) ((void)__android_log_print(ANDROID_LOG_WARN, "DecompressionTest", __VA_ARGS__))
#endif
#pragma once

#include "Common.h"

#if defined(FL_PLATFORM_ANDROID)
#include "fl/platform/android/AndroidSystem.h"
#endif

namespace fl {

#if defined(FL_PLATFORM_WINDOWS)
	void Init();
#elif defined(FL_PLATFORM_ANDROID)
	void Init(void* env, void* mainView);
#endif

}
#include "Core.h"

#include <iostream>
#include "gl.h"

#if defined(FL_PLATFORM_ANDROID)
#include "platform/android/AndroidSystem.h"
#endif

namespace fl {

#if defined(FL_PLATFORM_WINDOWS)

	void Init()
	{
	}

#elif defined(FL_PLATFORM_ANDROID)

	void Init(void* env, void* mainView)
	{
		AndroidSystem::Init((JNIEnv*)env, (jobject)mainView);
	}

#endif

	inline GLenum GLCheckError()
	{
		return glGetError();
	}

	void GLClearError()
	{
		GLCheckError();
	}

	bool GLLogCall(const char* function, const char* file, int line)
	{
		while (GLenum error = GLCheckError())
		{
			std::cout << "[OpenGL Error] (" << error << "): " << function << " " << file <<  ":" << line << std::endl;
			return false;
		}
		return true;
	}

}
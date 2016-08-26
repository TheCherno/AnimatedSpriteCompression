#pragma once

#if defined(FL_PLATFORM_WINDOWS)
#include <GL/glew.h>
#elif defined(FL_PLATFORM_ANDROID)
#include <GLES2/gl2.h>
#else
#error No platform defined!
#endif
#pragma once

#include <jni.h>

namespace fl {

	class AndroidSystem
	{
	public:
		static void Init(JNIEnv* env, jobject mainView);
	};

}

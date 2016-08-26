#include "AndroidSystem.h"

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>

namespace fl {

	JNIEnv* g_Env;
	jobject g_MainView;
	AAssetManager* g_AssetManager = nullptr;

	void AndroidSystem::Init(JNIEnv* env, jobject mainView)
	{
		g_Env = env;
		g_MainView = mainView;

		// Init AssetManager
		jmethodID getAssets = env->GetMethodID(env->GetObjectClass(mainView), "getAssets", "()Landroid/content/res/AssetManager;");
		g_AssetManager = AAssetManager_fromJava(env, env->CallObjectMethod(mainView, getAssets));
	}

}
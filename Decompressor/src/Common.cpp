#include "Common.h"

#if defined(PLATFORM_WIN32)
void Init()
{

}

byte* ReadFile(const String& path, size_t* outSize)
{
	FILE* file = fopen(path.c_str(), "rb");
	if (!file)
		return nullptr;

	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);
	fseek(file, 0, SEEK_SET);

	if (outSize)
		*outSize = size;

	byte* buffer = new byte[size];
	fread(buffer, 1, size, file);
	fclose(file);
	return buffer;
}

#elif defined(PLATFORM_ANDROID)

static JNIEnv* s_Env;
static jobject s_MainView;
static AAssetManager* s_AssetManager = nullptr;

void Init(JNIEnv* env, jobject mainView)
{
	s_Env = env;
	s_MainView = mainView;

	// Init AssetManager
	jmethodID getAssets = env->GetMethodID(env->GetObjectClass(mainView), "getAssets", "()Landroid/content/res/AssetManager;");
	s_AssetManager = AAssetManager_fromJava(env, env->CallObjectMethod(mainView, getAssets));
}

byte* ReadFile(const String& path, size_t* outSize)
{
	ASSERT(s_AssetManager);

	LOG("Reading file '%s'...", path.c_str());
	AAsset* file = AAssetManager_open(s_AssetManager, path.c_str(), AASSET_MODE_BUFFER);
	if (!file)
		return nullptr;

	size_t size = AAsset_getLength(file);
	if (outSize)
		*outSize = size;

	byte* result = new byte[size];
	AAsset_read(file, result, size);
	return result;
}

#else
#error No platform defined!
#endif

#include "fl/system/FileSystem.h"

#include <algorithm>

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>

namespace fl {

	extern JNIEnv* g_Env;
	extern jobject g_MainView;
	extern AAssetManager* g_AssetManager;

	bool FileSystem::FileExists(const String& path)
	{
		return false;
	}

	int FileSystem::GetFileSize(const String& path)
	{
		return 0;
	}

	byte* FileSystem::ReadFile(const String& path, size_t* outSize)
	{
		FL_ASSERT(g_AssetManager);

		FL_LOG("Reading file '%s'...", path.c_str());
		AAsset* file = AAssetManager_open(g_AssetManager, path.c_str(), AASSET_MODE_BUFFER);
		FL_ASSERT(file);
		if (!file)
			return nullptr;

		size_t size = AAsset_getLength(file);
		if (outSize)
			*outSize = size;

		byte* result = new byte[size];
		AAsset_read(file, result, size);
		AAsset_close(file);
		return result;
	}

	bool FileSystem::ReadFile(const String& path, void* buffer, long long size)
	{
		return false;
	}

	String FileSystem::ReadTextFile(const String& path)
	{
		FL_ASSERT(g_AssetManager);

		FL_LOG("Reading file '%s'...", path.c_str());
		AAsset* file = AAssetManager_open(g_AssetManager, path.c_str(), AASSET_MODE_BUFFER);

		if (!file)
			return "";

		size_t size = AAsset_getLength(file);
		FL_ASSERT(size < 100 * 1024 * 1024);

		String result(size, 0);
		AAsset_read(file, &result[0], size);
		AAsset_close(file);

		// Strip carriage returns
		result.erase(std::remove(result.begin(), result.end(), '\r'), result.end());
		return result;
	}

	bool FileSystem::WriteFile(const String& path, byte* buffer)
	{
		return false;
	}

	bool FileSystem::WriteTextFile(const String& path, const String& text)
	{
		return false;
	}

}
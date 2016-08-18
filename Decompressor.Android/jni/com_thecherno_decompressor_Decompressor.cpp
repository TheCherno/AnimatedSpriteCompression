#include "com_thecherno_decompressor_Decompressor.h"
#include "DecompressionTest.h"

JNIEXPORT jfloatArray JNICALL Java_com_thecherno_decompressor_Decompressor_RunDecompression(JNIEnv* env, jobject obj)
{
	Init(env, obj);

	const int count = 14 * 2;
	jfloatArray result = env->NewFloatArray(count);
	if (!result)
		return nullptr;

	jfloat data[count];
	
	DecompressionTest tester;
	std::vector<DecompressionResult> results = tester.RunAllTests();
	for (int i = 0; i < results.size(); i++)
	{
		data[i * 2 + 0] = (float)results[i].size;
		data[i * 2 + 1] = results[i].time;
	}

	env->SetFloatArrayRegion(result, 0, count, data);
	return result;
}
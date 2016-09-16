#include "com_thecherno_decompressor_Decompressor.h"
#include "DecompressionTest.h"

using namespace fl;

extern void UpdateAnimation(Animation* animation);
extern Renderer* s_Renderer;

JNIEXPORT jfloatArray JNICALL Java_com_thecherno_decompressor_Decompressor_RunDecompression(JNIEnv* env, jobject obj)
{
	Init(env, obj);
	s_Renderer = new Renderer();

	DecompressionTest tester;
	std::vector<DecompressionResult> results = tester.RunAllTests();

	Decompressor d("butterfly512-uncompressed.bin");
	UpdateAnimation(d.Decompress3());

	uint count = results.size() * 2;
	jfloatArray result = env->NewFloatArray(count);
	if (!result)
		return nullptr;

	jfloat* data = new jfloat[count];
	for (int i = 0; i < results.size(); i++)
	{
		data[i * 2 + 0] = (float)results[i].size;
		data[i * 2 + 1] = results[i].time;
	}

	env->SetFloatArrayRegion(result, 0, count, data);
	delete[] data;
	return result;
}

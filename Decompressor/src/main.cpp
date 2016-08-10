#include "Common.h"
#include <Windows.h>

#include "Decompressor.h"

class Timer
{
private:
	LARGE_INTEGER m_Start;
	double m_Frequency;
public:
	Timer()
	{
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);
		m_Frequency = 1.0 / frequency.QuadPart;

		Reset();
	}

	void Reset()
	{
		QueryPerformanceCounter(&m_Start);
	}

	float Elapsed()
	{
		LARGE_INTEGER current;
		QueryPerformanceCounter(&current);
		LONGLONG cycles = current.QuadPart - m_Start.QuadPart;
		return (float)(cycles * m_Frequency);
	}

	float ElapsedMillis()
	{
		return Elapsed() * 1000.0f;
	}
};

unsigned char stbbuffer[1024 * 1024 * 10];
unsigned char* stbtop = stbbuffer;
#define STBI_MALLOC(x) stbtop; stbtop += x;
#define STBI_FREE(x)
#define STBI_REALLOC(x, y) stbtop; if (x) memcpy(stbtop, x, y); stbtop += y;

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void ReadPNG(byte* png, long bsize)
{
	stbtop = stbbuffer;
	int x, y;
	stbi_load_from_memory((stbi_uc*)png, bsize, &x, &y, nullptr, 0);
}

int main()
{
	{
		float time = 0.0f;
		byte* buffer[128];
		long bsize[128];
		for (int i = 0; i < 128; i++)
		{
			char filename[128];
			sprintf(filename, "control/%04d.png", i + 1);
			buffer[i] = ReadFile(filename, &bsize[i]);
		}
		Timer timer;
		for (int i = 0; i < 1; i++)
		{
			for (int j = 0; j < 128; j++)
			{
				if (buffer[j] == 0)
					break;
				ReadPNG(buffer[j], bsize[j]);
			}
		}
		time += timer.ElapsedMillis();
		std::cout << time << "ms" << std::endl;
	}

	{
		float time = 0.0f;
		Decompressor decompressor("animation.bin");
		Timer timer;
		for (int i = 0; i < 1000; i++)
		{
			decompressor.Decompress();
		}
		time += timer.ElapsedMillis();
		std::cout << time << "ms" << std::endl;
	}

	{
		float time = 0.0f;
		Decompressor decompressor("animation2.bin");
		Timer timer;
		for (int i = 0; i < 1000; i++)
		{
			decompressor.Decompress2();
		}
		time += timer.ElapsedMillis();
		std::cout << time << "ms" << std::endl;
	}

	{
		float time = 0.0f;
		Decompressor decompressor("animation3.bin");
		Timer timer;
		for (int i = 0; i < 1000; i++)
		{
			decompressor.Decompress2();
		}
		time += timer.ElapsedMillis();
		std::cout << time << "ms" << std::endl;
	}

	system("PAUSE");
	return 0;
}
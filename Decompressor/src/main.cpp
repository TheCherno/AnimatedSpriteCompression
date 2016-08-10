#include <iostream>
#include <vector>
#include <Windows.h>

typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned int uint;

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

char staticbuffer[1024 * 1024 * 10];

byte* ReadFile(const char* path, long* outsize = NULL)
{
	FILE* file = fopen(path, "rb");
	if (file == nullptr)
		return 0;
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	fseek(file, 0, SEEK_SET);

	if (outsize)
		*outsize = size;

	byte* buffer = new byte[size];
	fread(buffer, 1, size, file);
	fclose(file);
	return buffer;
}

inline int GetBPC(byte format)
{
	if (format == 0) // ARGB
		return 4;

	return -1;
}

int* ReconstructFrame(int* src, int* delta, int size)
{
	int* result = new int[size];
	for (int i = 0; i < size; i++)
		result[i] = src[i] + delta[i];

	return result;
}

void Deserialize(byte* buffer, long size)
{
	struct Header
	{
		byte h0, h1;
		byte quality;
		byte format;
		byte r0, r1;
		ushort frames;
		ushort width, height;
	};

	const Header& header = *(Header*)buffer;
	buffer += sizeof(Header);
	int frame = 0;

	int bpc = GetBPC(header.format);
	while (buffer)
	{
		byte frameType = *(byte*)buffer++;
		byte compressionType = *(byte*)buffer++;

		if (frameType == 0) // Keyframe
		{
			// Assume no compression for now
			int* pixels = (int*)staticbuffer;
			memcpy(pixels, buffer, header.width * header.height * bpc);
			buffer += header.width * header.height * bpc;
		}
		else // Delta frame
		{
			int* pixels = (int*)staticbuffer;
			int* currentPixel = pixels;
			int* end = pixels + header.width * header.height;
			while (currentPixel < end)
			{
				ushort nextStep = *(ushort*)buffer;
				buffer += 2;
				int count = nextStep & 0x7FFF;
				if (nextStep & 0x8000)
				{
					memcpy(currentPixel, buffer, count * bpc);
					currentPixel += count;
					buffer += count * bpc;
				}
				else
				{
					currentPixel += count;
				}
			}
		}

		++frame;
		if (frame >= header.frames)
			break;
	}
	// std::cout << "Deserialization took " << serializationTimer.ElapsedMillis() << " ms." << std::endl;
	// std::cout << "Done." << std::endl;
}

void Deserialize2(byte* buffer, long size)
{
	struct Header
	{
		byte h0, h1;
		byte quality;
		byte format;
		byte r0, r1;
		ushort frames;
		ushort width, height;
	};

	const Header& header = *(Header*)buffer;
	buffer += sizeof(Header);

	int bpc = GetBPC(header.format);
	int frame = 0;
	while (buffer)
	{
		byte frameType = *(byte*)buffer++;
		byte compressionType = *(byte*)buffer++;

		if (frameType == 0) // Keyframe
		{
			// Assume no compression for now
			int* pixels = (int*)staticbuffer;
			memcpy(pixels, buffer, header.width * header.height * bpc);
			buffer += header.width * header.height * bpc;
		}
		else // Delta frame
		{
			int* pixels = (int*)staticbuffer;
			int* currentPixel = pixels;
			int* end = pixels + header.width * header.height;
			while (currentPixel < end)
			{
				ushort skipcount = *(ushort*)&buffer[0];
				ushort copycount = *(ushort*)&buffer[2];
				buffer += 4;
				memcpy(currentPixel + skipcount, buffer, copycount * bpc);
				buffer += copycount * bpc;
				currentPixel += skipcount + copycount;
			}
		}

		++frame;
		if (frame >= header.frames)
			break;
	}
	// std::cout << "Deserialization (branchless) took " << serializationTimer.ElapsedMillis() << " ms." << std::endl;
	// std::cout << "Done." << std::endl;
}

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
		byte* buffer = ReadFile("animation.bin");
		Timer timer;
		for (int i = 0; i < 1000; i++)
		{
			Deserialize(buffer, 0);
		}
		time += timer.ElapsedMillis();
		std::cout << time << "ms" << std::endl;
	}

	{
		float time = 0.0f;
		byte* buffer = ReadFile("animation2.bin");
		Timer timer;
		for (int i = 0; i < 1000; i++)
		{
			Deserialize2(buffer, 0);
		}
		time += timer.ElapsedMillis();
		std::cout << time << "ms" << std::endl;
	}

	{
		float time = 0.0f;
		byte* buffer = ReadFile("animation3.bin");
		Timer timer;
		for (int i = 0; i < 1000; i++)
		{
			Deserialize2(buffer, 0);
		}
		time += timer.ElapsedMillis();
		std::cout << time << "ms" << std::endl;
	}

	system("PAUSE");
	return 0;
}
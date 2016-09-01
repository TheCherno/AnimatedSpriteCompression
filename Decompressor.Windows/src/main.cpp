#include "Common.h"

#include "Decompressor.h"
#include "DecompressionTest.h"

#include "Flinty.h"
#include <fl/gl.h>

// Extra stuff for stb
static unsigned char stbbuffer[1024 * 1024 * 10];
static unsigned char* stbtop = stbbuffer;
#define STBI_MALLOC(x) stbtop; stbtop += x;
#define STBI_FREE(x)
#define STBI_REALLOC(x, y) stbtop; if (x) memcpy(stbtop, x, y); stbtop += y;
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace fl;

static Window* s_Window;
static Shader* s_Shader;
static Animation* s_Animation;
static int s_Frame = 0;
static int s_Timer = 0;

static void OnUpdate()
{
	//std::cout << s_Window->GetFrameTime() << std::endl;

	s_Timer++;
	if (s_Animation && s_Timer % 2 == 0)
	{
		s_Frame = (s_Frame + 1) % s_Animation->frames;
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, s_Animation->width, s_Animation->height, GL_RGBA, GL_UNSIGNED_BYTE, s_Animation->data[s_Frame]);
	}
}

static void OnRender(Renderer& renderer)
{
	renderer.SetClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

static void OnKeyEvent(int key, int action)
{
}


void ReadPNG(byte* png, long bsize)
{
	stbtop = stbbuffer;
	int x, y;
	stbi_load_from_memory((stbi_uc*)png, bsize, &x, &y, nullptr, 0);
}

int main()
{
	Window window("Sandbox", 1280, 720);
	s_Window = &window;

	window.SetUpdateCallback(OnUpdate);
	window.SetRenderCallback(OnRender);
	window.SetKeyCallback(OnKeyEvent);

	s_Shader = ShaderFactory::SimpleShader();
	ShaderManager::Add(s_Shader);
	s_Shader->Bind();
	s_Shader->SetUniform("pr_matrix", (byte*)&maths::mat4::Orthographic(-16.0f, 16.0f, -9.0f, 9.0f, -1.0f, 1.0f));

	float vertices[] = {
		-4.5f, -4.5f, 0.0f, 0.0f, 1.0f,
		-4.5f,  4.5f, 0.0f, 0.0f, 0.0f,
		4.5f,  4.5f, 0.0f, 1.0f, 0.0f,
		4.5f, -4.5f, 0.0f, 1.0f, 1.0f
	};

	int indices[] = {
		0, 1, 2, 2, 3, 0
	};

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	uint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	uint ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (const void*)(3 * sizeof(float)));

#if 0
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

#endif

	DecompressionTest test;
	std::vector<DecompressionResult> results = test.RunAllTests();
	for (int i = 0; i < results.size(); i++)
	{
		FL_LOG("%d: %.4fms (%d bytes)", i, results[i].time, results[i].size);
	}


	Animation* anim;
	{
		float time = 0.0f;
		Decompressor decompressor("animation-lz4.bin");
		Timer timer;
		anim = decompressor.Decompress();
		time += timer.ElapsedMillis();
		std::cout << time << "ms" << std::endl;
	}

	s_Animation = anim;

	uint texture;
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, anim->width, anim->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	window.Show();

	return 0;
}
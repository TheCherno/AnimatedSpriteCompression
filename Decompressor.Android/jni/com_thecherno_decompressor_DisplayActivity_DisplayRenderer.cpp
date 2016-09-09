#include "com_thecherno_decompressor_DisplayActivity_DisplayRenderer.h"

#include "Decompressor.h"

#include <Flinty.h>
#include <fl/gl.h>

using namespace fl;

static Shader* s_Shader;
static int s_Frame = 0;
static int s_Time = 0;

static float s_FrameTime = 0.0f;
static float s_UpdateTimer = 0.0f;
static float s_UpdateTick = 1.0f / 60.0f;
static uint s_Frames = 0;
static uint s_Updates = 0;
static Timer s_Timer;
static float s_SecondTimer = 0.0f;

static Renderer* s_Renderer;

static Animation* s_Animation = nullptr;
static uint s_TextureID = 0;

static void OnUpdate()
{
	s_Time++;

	if (s_Animation && s_Time % 2 == 0)
	{
		s_Frame = (s_Frame + 1) % s_Animation->frames;
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, s_Animation->width, s_Animation->height, GL_RGBA, GL_UNSIGNED_BYTE, s_Animation->data[s_Frame]);
	}
}

static void OnRender(Renderer& renderer)
{
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

void UpdateAnimation(Animation* animation)
{
	SUBMIT_RENDER_3(Animation*&, m_Animation, s_Animation,
					Animation*, m_NewAnimation, animation,
					uint&, m_TextureID, s_TextureID, { 
		if (m_Animation)
			delete m_Animation;

		m_Animation = m_NewAnimation;

		if (m_TextureID)
			glDeleteTextures(1, &m_TextureID);

		glGenTextures(1, &m_TextureID);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_TextureID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
 		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_NewAnimation->width, m_NewAnimation->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	});
}

JNIEXPORT void JNICALL Java_com_thecherno_decompressor_DisplayActivity_00024DisplayRenderer_OnInit(JNIEnv *, jclass)
{
	s_Renderer = new Renderer();

	s_Shader = ShaderFactory::SimpleShader();
	ShaderManager::Add(s_Shader);
	s_Shader->Bind();
	maths::mat4 proj = maths::mat4::Orthographic(-9.0f, 9.0f, -16.0f, 16.0f, -1.0f, 1.0f);
	s_Shader->SetUniform("pr_matrix", (byte*)&proj);

	float vertices[] = {
		-8.5f, -8.5f, 0.0f, 0.0f, 1.0f,
		-8.5f,  8.5f, 0.0f, 0.0f, 0.0f,
		8.5f,  8.5f, 0.0f, 1.0f, 0.0f,
		8.5f, -8.5f, 0.0f, 1.0f, 1.0f
	};

	int indices[] = {
		0, 1, 2, 2, 3, 0
	};


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
}

JNIEXPORT void JNICALL Java_com_thecherno_decompressor_DisplayActivity_00024DisplayRenderer_OnSurfaceChanged(JNIEnv *, jclass, jint width, jint height)
{
	float w = 9.0f;
	float h = 16.0f;
	if (width > height)
	{
		w = 16.0f;
		h = 9.0f;
	}

	maths::mat4 proj = maths::mat4::Orthographic(-w, w, -h, h, -1.0f, 1.0f);
	s_Shader->SetUniform("pr_matrix", (byte*)&proj);
}

JNIEXPORT void JNICALL Java_com_thecherno_decompressor_DisplayActivity_00024DisplayRenderer_OnDraw(JNIEnv *, jclass)
{
	if (s_SecondTimer == 0.0f)
		s_SecondTimer = s_Timer.Elapsed();

	float now = s_Timer.ElapsedMillis();
	s_Renderer->Clear();

	if (now - s_UpdateTimer > s_UpdateTick)
	{
		OnUpdate();
		s_Updates++;
		s_UpdateTimer += s_UpdateTick;
	}

	{
		Timer frametime;
		OnRender(*s_Renderer);
		s_Renderer->Flush();
		s_Frames++;
		s_FrameTime = frametime.ElapsedMillis();
	}

	if (s_Timer.Elapsed() - s_SecondTimer > 1.0f)
	{
		s_SecondTimer++;
		FL_LOG("Perf: %d ups, %d fps", s_Updates, s_Frames);
		s_Updates = 0;
		s_Frames = 0;
	}
}

#include "Renderer.h"

#include <iostream>

#if defined(FL_PLATFORM_WINDOWS)
#include <GL/glew.h>
#elif defined(FL_PLATFORM_ANDROID)
#include <GLES2/gl2.h>
#endif

namespace fl {

	Renderer* Renderer::s_Instance = nullptr;

	Renderer::Renderer()
	{
		Init();
		s_Instance = this;
	}

	void Renderer::Submit(RenderCommand* command)
	{
		m_CommandBuffer.push(command);
	}

	void Renderer::SetClearColor(float r, float g, float b, float a)
	{
		GLCall(glClearColor(r, g, b, a));
	}

	void Renderer::Clear()
	{
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	}

	void Renderer::Flush()
	{
		while (!m_CommandBuffer.empty())
		{
			RenderCommand* command = m_CommandBuffer.front();
			command->OnExecute();
			delete command;
			m_CommandBuffer.pop();
		}
	}

}
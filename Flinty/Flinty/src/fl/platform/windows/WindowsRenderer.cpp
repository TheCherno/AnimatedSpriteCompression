#include "fl/graphics/Renderer.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace fl {

	void Renderer::Init()
	{
		if (glewInit() != GLEW_OK)
		{
			std::cout << "Unable to initialize GLEW!" << std::endl;
			return;
		}

	}

}
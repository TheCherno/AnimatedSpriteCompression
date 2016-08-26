#include "fl/graphics/Renderer.h"

#include "fl/gl.h"

namespace fl {

	void Renderer::Init()
	{
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

}
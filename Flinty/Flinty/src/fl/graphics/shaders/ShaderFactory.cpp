#include "ShaderFactory.h"

namespace fl { namespace ShaderFactory {

	static const char* s_SimpleShader =
#if defined(FL_PLATFORM_WINDOWS)
		#include "default/Simple.shader"
#elif defined(FL_PLATFORM_ANDROID)
		"#shader vertex\n"
		"#version 100\n"
		"\n"
		"precision highp float;\n"
		"\n"
		"attribute vec4 position;\n"
		"attribute vec2 uv;\n"
		"attribute vec2 mask_uv;\n"
		"\n"
		"uniform mat4 pr_matrix;\n"
		"\n"
		"varying vec2 var_UV;\n"
		"\n"
		"void main()\n"
		"{\n"
		"	gl_Position = pr_matrix * position;\n"
		"	var_UV = uv;\n"
		"}\n"
		"\n"
		"#shader fragment\n"
		"#version 100\n"
		"\n"
		"precision highp float;\n"
		"\n"
		"uniform sampler2D u_Texture;\n"
		"\n"
		"varying vec2 var_UV;\n"
		"\n"
		"void main()\n"
		"{\n"
		"	gl_FragColor = texture2D(u_Texture, var_UV);\n"
		"}\n"
#endif
		;

	Shader* SimpleShader()
	{
		return Shader::CreateFromSource("Simple Shader", s_SimpleShader);
	}

} }
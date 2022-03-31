#pragma once
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <vector>

class CGameWindow;

namespace GL
{
	using Shader = GLuint;

	struct Object
	{
		GLuint vbo;
		GLuint vao;
		GLuint ebo;
		Shader shader;
		glm::mat4 transform;
	};

	struct RendererInternalData
	{
		std::vector<Object> meshes;
		std::vector<Shader> shaders;
	};
}

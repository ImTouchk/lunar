#pragma once
#include "utils/identifier.hpp"
#include "render/mesh.hpp"

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <vector>

class CGameWindow;

namespace GL
{
	using Shader = GLuint;

	struct DrawableObjectData
	{
		Identifier identifier;
		MeshType type;
		ShaderWrapper shader;
		GLuint indices;
		GLuint vao;
		GLuint vbo;
		GLuint ebo;
		glm::mat4 transform;
	};

	struct RendererInternalData
	{
		std::vector<DrawableObjectData> meshes;
		std::vector<Shader> shaders;
	};
}

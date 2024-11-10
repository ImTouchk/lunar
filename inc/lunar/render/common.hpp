#pragma once
#include <lunar/api.hpp>
#include <glm/glm.hpp>

namespace Render
{
	struct LUNAR_API Vertex
	{
		glm::vec3 position;
		float uv_x;
		glm::vec3 normal;
		float uv_y;
		glm::vec4 color;
	};

	struct LUNAR_API UniformBufferData
	{
		glm::mat4 view;
		glm::mat4 projection;
		glm::mat4 viewproj;
		glm::vec4 ambientColor;
		glm::vec4 sunlightDir;
		glm::vec4 sunlightCol;
	};
}

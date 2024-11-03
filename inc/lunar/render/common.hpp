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
}

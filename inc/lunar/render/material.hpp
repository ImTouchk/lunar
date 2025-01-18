#pragma once
#include <lunar/api.hpp>
#include <lunar/render/texture.hpp>

namespace Render
{
	struct LUNAR_API Material
	{
		Texture albedo    = {};
		float   metallic  = 0.2f;
		float   roughness = 0.2f;
		float   ao        = 0.1f;
	};
}

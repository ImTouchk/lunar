#pragma once
#include <lunar/api.hpp>
#include <lunar/render/texture.hpp>

namespace Render
{
	struct LUNAR_API Material
	{
		Texture colorMap = {};
	};
}


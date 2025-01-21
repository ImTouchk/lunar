#pragma once
#include <lunar/render/common.hpp>
#include <lunar/render/imp.hpp>

#include <glad/gl.h>

namespace lunar::Render
{
	namespace imp
	{
		struct WindowBackendData
		{
			GLuint globalVao = 0;
		};
	}
}

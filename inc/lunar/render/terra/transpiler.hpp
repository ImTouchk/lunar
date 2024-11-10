#pragma once
#include <lunar/api.hpp>
#include <lunar/file/filesystem.hpp>

namespace Terra
{
	namespace imp
	{
		class LUNAR_API VulkanGLSLTranspiler
		{

		};
	}

	enum class LUNAR_API TranspilerOutput
	{
		eVulkanGLSL
	};

	bool LUNAR_API transpileCode(const Fs::Path& path, TranspilerOutput output);
}

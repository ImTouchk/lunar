#pragma once
#ifdef LUNAR_VULKAN
#	include <vulkan/vulkan.hpp>
#endif
#include <lunar/render/render_context.hpp>

namespace Render
{
	class Image
	{
	public:
		

	private:
		std::shared_ptr<RenderContext> context;
	};
}

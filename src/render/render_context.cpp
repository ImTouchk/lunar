#include <lunar/render/render_context.hpp>
#include <lunar/api.hpp>

#ifdef LUNAR_VULKAN
#	include <lunar/render/internal/render_vk.hpp>
#endif

namespace Render
{
	std::shared_ptr<RenderContext> createSharedContext()
	{
#ifdef LUNAR_VULKAN
		return std::make_shared<VulkanContext>();
#else
		throw;
#endif
	}
}

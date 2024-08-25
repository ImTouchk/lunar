#pragma once
#include <lunar/api.hpp>
#include <memory>

namespace Render
{
	class LUNAR_API RenderContext
	{
	public:
		virtual ~RenderContext() = default;

		virtual void init() = 0;
		virtual void destroy() = 0;
	};

	LUNAR_API std::shared_ptr<RenderContext> createSharedContext();
}

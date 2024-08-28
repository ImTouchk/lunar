#pragma once
#include <lunar/api.hpp>
#include <lunar/core/scene.hpp>
#include <lunar/render/render_target.hpp>
#include <concepts>
#include <memory>

namespace Render
{
	class LUNAR_API RenderContext
	{
	public:
		virtual ~RenderContext() = default;

		virtual void init() = 0;
		virtual void destroy() = 0;
		virtual void draw(Core::Scene& scene, RenderTarget* target) = 0;

		template<typename T> requires std::derived_from<T, RenderTarget>
		inline void draw(Core::Scene& scene, T& target)
		{
			draw(scene, reinterpret_cast<RenderTarget*>(&target));
		}
	};

	LUNAR_API std::shared_ptr<RenderContext> createSharedContext();
}

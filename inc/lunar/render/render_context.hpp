#pragma once
#include <lunar/api.hpp>
#include <lunar/core/scene.hpp>
#include <lunar/render/render_target.hpp>
#include <concepts>
#include <memory>

namespace Render
{
	class LUNAR_API Camera;
	class LUNAR_API RenderContext
	{
	public:
		virtual ~RenderContext() = default;

		virtual void init() = 0;
		virtual void destroy() = 0;
		virtual void draw(Core::Scene& scene, Camera& camera, RenderTarget* target) = 0;

		//virtual void draw(Core::Scene& scene, RenderTarget* target) = 0;

		template<typename T> requires IsRenderTarget<T>
		inline void draw(std::shared_ptr<Core::Scene>& scene, Camera& camera, T& target)
		{
			draw(*scene.get(), camera, &target);
		}
	};

	LUNAR_API std::shared_ptr<RenderContext> CreateDefaultContext();
}

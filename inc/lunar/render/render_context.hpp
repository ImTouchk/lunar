#pragma once
#include <lunar/api.hpp>
#include <lunar/core/scene.hpp>
#include <lunar/render/render_target.hpp>
#include <concepts>
#include <memory>

#ifdef LUNAR_IMGUI
#	include <imgui.h>
#endif

namespace Render
{
	class LUNAR_API Camera;
	class LUNAR_API RenderContext
	{
	public:
		virtual ~RenderContext() = default;

		virtual void init() = 0;
		virtual void destroy() = 0;
		virtual void draw(Core::Scene& scene, Camera& camera) = 0;
		virtual void begin(RenderTarget* target) = 0;
		virtual void end()   = 0;

		//virtual void draw(Core::Scene& scene, RenderTarget* target) = 0;

		template<typename T> requires IsRenderTarget<T>
		inline void begin(T& target) { begin(&target); }

		inline void draw(std::shared_ptr<Core::Scene>& scene, Camera& camera) { draw(*scene.get(), camera); }
		inline void draw(std::shared_ptr<Core::Scene>& scene)
		{
			auto* camera = scene->getMainCamera();
			DEBUG_ASSERT(camera != nullptr, "Scene does not have a main camera set.");
			draw(scene, *camera);
		}

		RenderTarget* getCurrentTarget()
		{
			DEBUG_ASSERT(_currentTarget != nullptr, "Frame drawing has not begun yet.");
			return _currentTarget;
		}

#ifdef LUNAR_IMGUI
		ImGuiContext* getImGuiContext() { return _imguiContext; }

	protected:
		ImGuiContext* _imguiContext  = nullptr;
		RenderTarget* _currentTarget = nullptr;
#endif
	};

	LUNAR_API std::shared_ptr<RenderContext> CreateDefaultContext();
}

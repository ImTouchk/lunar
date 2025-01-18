#pragma once
#include <lunar/api.hpp>
#include <lunar/core/scene.hpp>
#include <lunar/render/render_context.hpp>

namespace Core { class InputHandler;  }

namespace Debug
{
	LUNAR_API void DrawSceneHierarchyPanel(Render::RenderContext& context, Core::Scene& scene);
	LUNAR_API void DrawGeneralInfoPanel(Render::RenderContext& context);
}
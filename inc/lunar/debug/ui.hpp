#pragma once
#include <lunar/api.hpp>
#include <lunar/core/scene.hpp>
#include <lunar/render/render_context.hpp>

namespace Debug
{
	LUNAR_API void DrawSceneHierarchyPanel(Core::Scene& scene, Render::RenderContext& context);
	LUNAR_API void DrawGeneralInfoPanel(Render::RenderContext& context);
}
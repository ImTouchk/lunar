#pragma once
#include <lunar/api.hpp>
#include <lunar/core/scene.hpp>
#include <lunar/render/render_target.hpp>
#include <lunar/render/common.hpp>
#include <concepts>
#include <memory>

#ifdef LUNAR_IMGUI
#	include <imgui.h>
#endif

namespace Render
{
	class LUNAR_API GraphicsShader;
	class LUNAR_API Mesh;
	class LUNAR_API Cubemap;
	class LUNAR_API Texture;
	class LUNAR_API Material;
	enum class LUNAR_API MeshPrimitive;

	class LUNAR_API Camera;
	class LUNAR_API RenderContext
	{
	public:
		virtual ~RenderContext() = default;

		virtual void init() = 0;
		virtual void destroy() = 0;
		virtual void draw(Core::Scene& scene) = 0;
		virtual void draw(const Cubemap& cubemap) = 0;
		virtual void draw(const Texture& texture) = 0;
		virtual void draw(const Mesh& mesh) = 0;
		virtual void draw(const Mesh& mesh, const GraphicsShader& shader, const Material& material) = 0;
		virtual void clear(float r, float g, float b, float a) = 0;
		virtual void begin(RenderTarget* target)  = 0;
		virtual void end()   = 0;

		RenderTarget* getCurrentTarget();
		void          setCamera(const Camera& camera);
		const Mesh&   getPrimitiveMesh(MeshPrimitive primitive) const;

		//virtual void draw(Core::Scene& scene, RenderTarget* target) = 0;

		template<typename T> requires IsRenderTarget<T>
		inline void begin(T& target) { begin(&target); }
		inline void draw(std::shared_ptr<Core::Scene>& scene) { draw(*scene.get()); }

#ifdef LUNAR_IMGUI
		ImGuiContext* getImGuiContext();
#endif

	protected:
		void            loadPrimitives();
		void            unloadPrimitives();
		SceneLightData& getSceneLightData(Core::Scene& scene);
		Mesh&           _getPrimitiveMesh(MeshPrimitive primitive);

#ifdef LUNAR_IMGUI
		ImGuiContext*   imguiContext    = nullptr;
#endif
		RenderTarget*   currentTarget   = nullptr;
		Mesh*           primitiveMeshes = nullptr;
		GraphicsShader* skyboxShader    = nullptr;
		GraphicsShader* unlitShader     = nullptr;
		const Camera*   camera          = nullptr;
		const Cubemap*  cubemap         = nullptr;
		SceneLightData  sceneLightData  = {};
	};

	LUNAR_API std::shared_ptr<RenderContext> CreateDefaultContext();
}

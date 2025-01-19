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

#ifdef LUNAR_OPENGL
#include <glad/gl.h>
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
		RenderContext();
		~RenderContext();

		void init();
		void destroy();
		void draw(Core::Scene& scene);
		void draw(const Cubemap& cubemap);
		void draw(const Texture& texture);
		void draw(const Mesh& mesh);
		void draw(const Mesh& mesh, const GraphicsShader& shader, const Material& material);
		void clear(float r, float g, float b, float a);
		void begin(RenderTarget* target);
		void end();

		RenderTarget* getCurrentTarget();
		void          setCamera(const Camera& camera);
		void          loadMaterial(Material& material);
		const Mesh&   getPrimitiveMesh(MeshPrimitive primitive) const;

		//virtual void draw(Core::Scene& scene, RenderTarget* target) = 0;

		template<typename T> requires IsRenderTarget<T>
		inline void begin(T& target) { begin(&target); }
		inline void draw(std::shared_ptr<Core::Scene>& scene) { draw(*scene.get()); }

#ifdef LUNAR_IMGUI
		ImGuiContext* getImGuiContext();
#endif

#ifdef LUNAR_OPENGL
		GLuint glGetCaptureFramebuffer();
		GLuint glGetCaptureRenderbuffer();
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
		Material*       materials       = nullptr;
		const Camera*   camera          = nullptr;
		const Cubemap*  cubemap         = nullptr;
		SceneLightData  sceneLightData  = {};

#ifdef LUNAR_OPENGL
		GLuint          captureFbo = 0;
		GLuint          captureRbo = 0;
		GLuint          sceneUbo   = 0;
		GLuint          lightsUbo  = 0;
#endif
	};

	LUNAR_API std::shared_ptr<RenderContext> CreateDefaultContext();
}

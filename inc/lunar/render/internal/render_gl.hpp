#pragma once
#include <lunar/render/render_context.hpp>
#include <lunar/api.hpp>
#include <glad/gl.h>

namespace Render
{
	class LUNAR_API GLContext : public RenderContext
	{
	public:
		GLContext();
		virtual ~GLContext() = default;

		void init() override;
		void destroy() override;
		void draw(Core::Scene& scene, Camera& camera) override;
		void begin(RenderTarget* target) override;
		void end()   override;

	private:
		GLuint ubo = 0;
	};

	namespace imp
	{
		enum class LUNAR_API TextureFormat
		{
			eUnknown = -1,
			eRGBA    = GL_RGBA,
		};
	}
}

#pragma once
#include <lunar/render/render_context.hpp>
#include <lunar/api.hpp>
#include <glad/gl.h>

namespace Render
{
	//class LUNAR_API GLContext : public RenderContext
	//{
	//public:
	//	GLContext();
	//	virtual ~GLContext() = default;

	//	void init()                       override;
	//	void destroy()                    override;
	//	void draw(Core::Scene& scene)     override;
	//	void draw(const Texture& texture) override;
	//	void draw(const Mesh& mesh)       override;
	//	void draw(const Mesh& mesh, const GraphicsShader& shader, const Material& material) override;
	//	void draw(const Cubemap& cubemap)              override;
	//	void clear(float r, float g, float b, float a) override;
	//	void begin(RenderTarget* target)               override;
	//	void end()                                     override;
	//	
	//	GLuint glGetCaptureFramebuffer();
	//	GLuint glGetCaptureRenderbuffer();

	//private:
	//	GLuint sceneUbo        = 0;
	//	GLuint lightsUbo       = 0;

	//	GLuint skyboxVao     = 0;
	//	GLuint skyboxVbo     = 0;
	//	GLuint skyboxEbo     = 0;
	//	GLuint captureFbo    = 0;
	//	GLuint captureRbo    = 0;
	//	GLuint irradianceMap = 0;
	//	GLuint fbo           = 0;
	//	GLuint rbo           = 0;
	//	
	//};

	namespace imp
	{
		enum class LUNAR_API TextureFormat
		{
			eUnknown = -1,
			eRGBA    = GL_RGBA,
			eRGB     = GL_RGB,
			eRGB16F  = GL_RGB16F
		};

		enum class LUNAR_API TextureByteFormat
		{
			eUnknown      = -1,
			eUnsignedByte = GL_UNSIGNED_BYTE,
			eFloat        = GL_FLOAT,
		};

		enum class LUNAR_API TextureWrapping
		{
			eUnknown     = -1,
			eClampToEdge = GL_CLAMP_TO_EDGE,
			eRepeat      = GL_REPEAT
		};
	}
}

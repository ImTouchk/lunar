#pragma once
#include <lunar/api.hpp>
#include <lunar/file/filesystem.hpp>
#include <lunar/render/common.hpp>
#include <lunar/render/cubemap.hpp>
#include <lunar/render/render_context.hpp>
#include <lunar/render/render_target.hpp>

#ifdef LUNAR_OPENGL
#	include <lunar/render/internal/render_gl.hpp>
#	include <glad/gl.h>
#endif

namespace Render
{
	enum class LUNAR_API TextureByteFormat
	{
		eUnknown      = imp::TextureByteFormat::eUnknown,
		eUnsignedByte = imp::TextureByteFormat::eUnsignedByte,
		eFloat        = imp::TextureByteFormat::eFloat
	};

	enum class LUNAR_API TextureFormat
	{
		eUnknown = imp::TextureFormat::eUnknown,
		eRGBA    = imp::TextureFormat::eRGBA,
		eRGB     = imp::TextureFormat::eRGB,
		eRGB16F  = imp::TextureFormat::eRGB16F
	};

	enum class LUNAR_API TextureFiltering
	{
		eNearest = 0,
		eLinear  = 1,
	};

	enum class LUNAR_API TextureWrapping
	{
		eUnknown     = imp::TextureWrapping::eUnknown,
		eClampToEdge = imp::TextureWrapping::eClampToEdge,
		eRepeat      = imp::TextureWrapping::eRepeat,
	};

	class LUNAR_API Texture : public RenderTarget
	{
	public:
		Texture() = default;

		void bind(size_t location);
		int getRenderWidth() const override;
		int getRenderHeight() const override;

#ifdef LUNAR_OPENGL
		GLuint glGetHandle();
#endif
	private:
#ifdef LUNAR_OPENGL
		GLuint _glHandle = 0;

		friend class GLContext;
#endif

		friend class TextureBuilder;
		friend class MeshRenderer;
		friend class GraphicsShader;
	};

	struct LUNAR_API TextureBuilder
	{
	public:
		TextureBuilder() = default;
		~TextureBuilder() = default;

		TextureBuilder& setSize(int width, int height);
		TextureBuilder& setDstFormat(TextureFormat format);
		TextureBuilder& setByteFormat(TextureByteFormat format);
		TextureBuilder& setFiltering(TextureFiltering filtering);
		TextureBuilder& setWrapping(TextureWrapping wrapping);
		TextureBuilder& fromImagePath(const Fs::Path& path);
		TextureBuilder& fromByteArray(TextureFormat originalFormat, int width, int height, void* data);
		TextureBuilder& useRenderContext(std::shared_ptr<RenderContext>& context);
		TextureBuilder& build();
		Texture getResult();
	private:
		Fs::Path                       path       = {};
		Texture                        result     = {};
		std::shared_ptr<RenderContext> context    = nullptr;
		int                            width      = 0;
		int                            height     = 0;
		int                            channels   = 0;
		void*                          rawBytes   = nullptr;
		TextureFormat                  srcFormat  = TextureFormat::eUnknown;
		TextureByteFormat              byteFormat = TextureByteFormat::eUnknown;
		TextureFormat                  dstFormat  = TextureFormat::eRGBA;
		TextureFiltering               filtering  = TextureFiltering::eLinear;
		TextureWrapping                wrapping   = TextureWrapping::eClampToEdge;

#ifdef LUNAR_OPENGL
		bool _glBuild();
#endif
	};
}

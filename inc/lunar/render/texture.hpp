#pragma once
#include <lunar/api.hpp>
#include <lunar/file/filesystem.hpp>
#include <lunar/render/common.hpp>
#include <lunar/render/render_context.hpp>

#ifdef LUNAR_OPENGL
#	include <lunar/render/internal/render_gl.hpp>
#	include <glad/gl.h>
#endif

namespace Render
{
	enum class LUNAR_API TextureFormat
	{
		eUnknown = imp::TextureFormat::eUnknown,
		eRGBA    = imp::TextureFormat::eRGBA,
	};

	enum class LUNAR_API TextureFiltering
	{
		eNearest = 0,
		eLinear  = 1,
	};

	class LUNAR_API Texture
	{
	public:
		Texture() = default;


	private:
#ifdef LUNAR_OPENGL
		GLuint _glHandle = 0;

		friend class GLContext;
#endif

		friend class TextureBuilder;
	};

	struct LUNAR_API TextureBuilder
	{
	public:
		TextureBuilder() = default;
		~TextureBuilder() = default;

		TextureBuilder& setFormat(TextureFormat format);
		TextureBuilder& setFiltering(TextureFiltering filtering);
		TextureBuilder& fromImagePath(const Fs::Path& path);
		TextureBuilder& fromByteArray(TextureFormat originalFormat, int width, int height, void* data);
		TextureBuilder& useRenderContext(std::shared_ptr<RenderContext>& context);
		TextureBuilder& build();
		Texture getResult();
	private:
		Fs::Path                       path      = {};
		Texture                        result    = {};
		std::shared_ptr<RenderContext> context   = nullptr;
		int                            width     = 0;
		int                            height    = 0;
		int                            channels  = 0;
		void*                          rawBytes  = nullptr;
		TextureFormat                  srcFormat = TextureFormat::eUnknown;
		TextureFormat                  dstFormat = TextureFormat::eRGBA;
		TextureFiltering               filtering = TextureFiltering::eLinear;

#ifdef LUNAR_OPENGL
		bool _glBuild();
#endif
	};
}

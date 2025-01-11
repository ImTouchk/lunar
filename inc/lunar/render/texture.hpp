#pragma once
#include <lunar/api.hpp>
#include <lunar/file/filesystem.hpp>
#include <lunar/render/common.hpp>
#include <lunar/render/render_context.hpp>

#ifdef LUNAR_OPENGL
#	include <glad/gl.h>
#endif

namespace Render
{
	enum class LUNAR_API TextureFormat
	{
		eUnknown = -1,
		eRGBA = 0,
	};

	class LUNAR_API Texture
	{
	public:
		Texture() = default;


	private:
#ifdef LUNAR_OPENGL
		GLuint _glHandle;

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

#ifdef LUNAR_OPENGL
		bool _glBuild();
#endif
	};
}

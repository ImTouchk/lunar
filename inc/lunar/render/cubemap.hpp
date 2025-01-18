#pragma once
#include <lunar/api.hpp>

#ifdef LUNAR_OPENGL
#	include <glad/GL.h>
#endif

namespace Render
{
	class LUNAR_API RenderContext;

	class LUNAR_API Cubemap
	{
	public:

	private:
		bool isHdr = false;

		friend struct CubemapBuilder;

#ifdef LUNAR_OPENGL
		GLuint _glHandle;
		GLuint _glIrradiance;
		GLuint _glPrefilter;
		GLuint _glBrdf;

		friend class GLContext;
#endif

		friend class GraphicsShader;
	};

	struct LUNAR_API CubemapBuilder
	{
	public:
		CubemapBuilder() = default;

		CubemapBuilder& fromHDRFile(const Fs::Path& path);
		CubemapBuilder& fromFiles(const std::initializer_list<Fs::Path>& files);
		CubemapBuilder& useRenderContext(std::shared_ptr<RenderContext>& context);
		CubemapBuilder& build();
		Cubemap getResult();

	private:
		std::shared_ptr<RenderContext> context     = nullptr;
		std::vector<Fs::Path>          files       = {};
		Cubemap                        result      = {};
		void*                          rawBytes[6] = {};
		int                            width[6]    = {};
		int                            height[6]   = {};
		int                            channels[6] = {};
		bool                           isHdr       = false;

		void buildIrradianceMap();
		void buildPrefilterMap();
		void buildBrdfTexture();
		void equirectToCubemap();
		void normalCubemap();

#ifdef LUNAR_OPENGL
		bool _glBuild();
#endif
	};
}

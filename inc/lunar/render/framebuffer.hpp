#pragma once
#include <lunar/api.hpp>
#include <lunar/render/render_context.hpp>

#ifdef LUNAR_OPENGL
#	include <glad/GL.h>
#endif

namespace Render
{
//	enum class LUNAR_API FramebufferUsage
//	{
//
//	};
//
//	class LUNAR_API Framebuffer
//	{
//	public:
//
//	private:
//#ifdef LUNAR_OPENGL
//		GLuint _glHandle = 0;
//
//		friend class GLContext;
//#endif
//
//		friend struct FramebufferBuilder;
//	};
//
//	struct LUNAR_API FramebufferBuilder
//	{
//		FramebufferBuilder() = default;
//
//		FramebufferBuilder& build();
//		Framebuffer 
//	private:
//		std::shared_ptr<RenderContext> context = nullptr;
//		Framebuffer                    result  = {};
//
//	};
}
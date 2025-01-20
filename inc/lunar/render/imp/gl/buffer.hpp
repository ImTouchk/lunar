#pragma once
#include <lunar/api.hpp>
#include <lunar/render/common.hpp>

#include <glad/gl.h>

namespace lunar::Render
{
	enum class LUNAR_API GpuBufferType : GLenum
	{
		eUnknown       = 0,
		eVertex        = GL_ARRAY_BUFFER,
		eIndex         = GL_ELEMENT_ARRAY_BUFFER,
		eUniform       = GL_UNIFORM_BUFFER,
		eShaderStorage = GL_SHADER_STORAGE_BUFFER,
	};

	enum class LUNAR_API GpuBufferUsageFlagBits : GLenum
	{
		eNone    = 0,
		eStatic  = GL_STATIC_DRAW,
		eDynamic = GL_DYNAMIC_DRAW
	};

	LUNAR_FLAGS(GpuBufferUsageFlags, GpuBufferUsageFlagBits);

	class LUNAR_API GpuBuffer_T
	{
	public:
		GpuBuffer_T
		(
			RenderContext_T* context,
			GpuBufferType type,
			GpuBufferUsageFlags usageFlags,
			size_t size,
			void* data
		) noexcept;
		GpuBuffer_T() noexcept = default;
		~GpuBuffer_T();

		GpuBufferType       getType()       const;
		GpuBufferUsageFlags getUsageFlags() const;
		size_t              getSize()       const;

		GLuint glGetHandle();

	private:
		GLuint              handle     = 0;
		GpuBufferType       type       = GpuBufferType::eUnknown;
		GpuBufferUsageFlags usageFlags = GpuBufferUsageFlagBits::eNone;
		size_t              size       = 0;
	};

	class LUNAR_API GpuVertexArrayObject_T
	{
	public:
		GpuVertexArrayObject_T(RenderContext_T* context) noexcept;
		GpuVertexArrayObject_T() noexcept = default;
		~GpuVertexArrayObject_T();

		void bind(GpuBuffer vertexBuffer, GpuBuffer indexBuffer);
		void bind();
		void unbind();

		GLuint handle = 0;
	};
}

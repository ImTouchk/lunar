#include <lunar/render/imp/gl/buffer.hpp>
#include <glad/gl.h>

namespace lunar::Render
{
	GpuBuffer_T::GpuBuffer_T
	(
		RenderContext_T* context, 
		GpuBufferType type, 
		GpuBufferUsageFlags usageFlags,
		size_t size, 
		void* data
	) noexcept
		: type(type),
		usageFlags(usageFlags),
		size(size)
	{
		glGenBuffers(1, &handle);
		glBindBuffer((GLenum)type, handle);
		glBufferData((GLenum)type, size, data, usageFlags);
		glBindBuffer((GLenum)type, 0);
	}

	GpuBuffer_T::~GpuBuffer_T()
	{
		if(handle != 0)
			glDeleteBuffers(1, &handle);
	}

	size_t GpuBuffer_T::getSize() const
	{
		return size;
	}

	GLuint GpuBuffer_T::glGetHandle()
	{
		return handle;
	}

	GpuVertexArrayObject_T::GpuVertexArrayObject_T(RenderContext_T* context) noexcept
	{
		glGenVertexArrays(1, &handle);
		glBindVertexArray(handle);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
		glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv_x));
		glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv_y));
		glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
		glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
		glEnableVertexAttribArray(5);
		glEnableVertexAttribArray(6);
		glBindVertexArray(0);
	}

	GpuVertexArrayObject_T::~GpuVertexArrayObject_T()
	{
		if (handle != 0)
			glDeleteVertexArrays(1, &handle);
	}

	void GpuVertexArrayObject_T::bind(GpuBuffer vertexBuffer, GpuBuffer indexBuffer)
	{
		glBindVertexArray(handle);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->glGetHandle());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer->glGetHandle());
		glBindVertexArray(0);
	}

	void GpuVertexArrayObject_T::bind()
	{
		glBindVertexArray(handle);
	}

	void GpuVertexArrayObject_T::unbind()
	{
		glBindVertexArray(0);
	}
}

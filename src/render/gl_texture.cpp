#include <glad/gl.h>
#include <GLFW/glfw3.h>

#ifdef LUNAR_IMGUI
#	include <imgui_impl_glfw.h>
#	include <imgui_impl_opengl3.h>
#endif

#include <lunar/render/render_context.hpp>
#include <lunar/render/render_components.hpp>
#include <lunar/render/internal/render_gl.hpp>
#include <lunar/render/window.hpp>
#include <lunar/render/shader.hpp>
#include <lunar/render/mesh.hpp>
#include <lunar/render/texture.hpp>
#include <lunar/file/text_file.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Render
{
	int Texture::getRenderWidth() const
	{
		GLint width;
		glGetTextureParameteriv(_glHandle, GL_TEXTURE_WIDTH, &width);
		return (int)width;
	}

	int Texture::getRenderHeight() const
	{
		GLint height;
		glGetTextureParameteriv(_glHandle, GL_TEXTURE_HEIGHT, &height);
		return (int)height;
	}

	void Texture::bind(size_t location)
	{
		glActiveTexture(GL_TEXTURE0 + location);
		glBindTexture(GL_TEXTURE_2D, _glHandle);
	}

	bool TextureBuilder::_glBuild()
	{
		auto& handle = result._glHandle;

		glGenTextures(1, &handle);
		glBindTexture(GL_TEXTURE_2D, handle);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (GLint)wrapping);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (GLint)wrapping);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, (GLint)wrapping);

		GLint min_filter = (filtering == TextureFiltering::eNearest) ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_LINEAR;
		GLint mag_filter = (filtering == TextureFiltering::eNearest) ? GL_NEAREST : GL_LINEAR;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
		
		if (rawBytes != nullptr)
			glTexImage2D(GL_TEXTURE_2D, 0, (GLint)dstFormat, width, height, 0, (GLint)srcFormat, (GLenum)byteFormat, rawBytes);
		else
			glTexImage2D(GL_TEXTURE_2D, 0, (GLint)dstFormat, width, height, 0, (GLint)srcFormat, (GLenum)byteFormat, nullptr);

		glGenerateMipmap(GL_TEXTURE_2D);

		return true;
	}
}
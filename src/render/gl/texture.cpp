#include <lunar/render/imp/gl/texture.hpp>
#include <lunar/utils/collections.hpp>
#include <lunar/debug/assert.hpp>
#include <lunar/debug/log.hpp>

namespace lunar::Render
{
	GpuTexture_T::GpuTexture_T
	(
		RenderContext_T*  context,
		int               width,
		int               height,
		void*             data,
		TextureFormat     srcFormat,
		TextureDataFormat dataFormat,
		TextureFormat     dstFormat,
		TextureType       type,
		TextureFiltering  minFiltering,
		TextureFiltering  magFiltering,
		TextureWrapping   wrapping,
		TextureFlags      flags
	) noexcept
		: width(width),
		height(height),
		format(dstFormat),
		type(type),
		minFiltering(minFiltering),
		magFiltering(magFiltering),
		wrapping(wrapping),
		flags(flags)
	{	
		glGenTextures(1, &handle);
		glBindTexture((GLenum)type, handle);
		glTexParameteri((GLenum)type, GL_TEXTURE_WRAP_S, (GLint)wrapping);
		glTexParameteri((GLenum)type, GL_TEXTURE_WRAP_T, (GLint)wrapping);
		glTexParameteri((GLenum)type, GL_TEXTURE_WRAP_R, (GLint)wrapping);
		glTexParameteri((GLenum)type, GL_TEXTURE_MIN_FILTER, (GLint)minFiltering);
		glTexParameteri((GLenum)type, GL_TEXTURE_MAG_FILTER, (GLint)magFiltering);

		if (type == TextureType::eCubemap)
		{
			for (size_t i = 0; i < 6; i++)
			{
				glTexImage2D(
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
					0, 
					(GLint)dstFormat, 
					width, 
					height, 
					0, 
					(GLint)srcFormat, 
					(GLenum)dataFormat, 
					data
				);
			}
		}
		else
		{
			glTexImage2D(
				(GLenum)type, 
				0, 
				(GLint)dstFormat, 
				width, 
				height, 
				0, 
				(GLint)srcFormat, 
				(GLenum)dataFormat, 
				data
			);
		}

		if (flags & TextureFlagBits::eBindless)
		{
			this->bindless = glGetTextureHandleARB(this->handle);
			glMakeTextureHandleResidentARB(this->bindless);
		}

		glBindTexture((GLenum)type, 0);
	}

	GpuTexture_T::~GpuTexture_T()
	{
		if (handle != 0)
			glDeleteTextures(1, &handle);
	}

	void GpuTexture_T::generateMipmaps()
	{
		glBindTexture((GLenum)type, handle);
		glGenerateMipmap((GLenum)type);
		glBindTexture((GLenum)type, 0);
	}

	int GpuTexture_T::getRenderWidth() const
	{
		return width;
	}

	int GpuTexture_T::getRenderHeight() const
	{
		return height;
	}

	GLuint GpuTexture_T::glGetHandle()
	{
		return handle;
	}

	GLuint64 GpuTexture_T::glGetBindlessHandle()
	{
		return bindless;
	}

	TextureType GpuTexture_T::getType() const
	{
		return type;
	}
}

#pragma once
#include <lunar/api.hpp>
#include <lunar/core/handle.hpp>
#include <lunar/render/render_target.hpp>
#include <lunar/render/imp.hpp>

#include <glad/gl.h>
#include <initializer_list>

namespace lunar::Render
{
	enum class LUNAR_API TextureFormat : GLenum
	{
		eUnknown = 0,
		eRG      = GL_RG,
		eRG16F   = GL_RG16F,
		eRGB     = GL_RGB,
		eRGBA    = GL_RGBA,
		eRGB16F  = GL_RGB16F,
	};

	enum class LUNAR_API TextureFiltering : GLenum
	{
		eUnknown = 0,
		eNearest,
		eLinear
	};

	enum class LUNAR_API TextureWrapping : GLint
	{
		eUnknown     = 0,
		eClampToEdge = GL_CLAMP_TO_EDGE,
		eRepeat      = GL_REPEAT
	};

	enum class LUNAR_API TextureDataFormat : GLenum
	{
		eUnknown      = 0,
		eUnsignedByte = GL_UNSIGNED_BYTE,
		eFloat        = GL_FLOAT
	};

	enum class LUNAR_API TextureType : GLenum
	{
		eUnknown = 0,
		e2D      = GL_TEXTURE_2D,
		eCubemap = GL_TEXTURE_CUBE_MAP
	};

	enum class LUNAR_API TextureFlagBits
	{
		eNone = 0
	};

	LUNAR_FLAGS(TextureFlags, TextureFlagBits);

	class LUNAR_API GpuTexture_T : public RenderTarget
	{
	public:
		GpuTexture_T
		(
			RenderContext_T*  context,
			int               width,
			int               height,
			void*             data,
			TextureFormat     srcFormat,
			TextureDataFormat dataFormat = TextureDataFormat::eUnsignedByte,
			TextureFormat     dstFormat  = TextureFormat::eRGBA,
			TextureType       type       = TextureType::e2D,
			TextureFiltering  filtering  = TextureFiltering::eLinear,
			TextureWrapping   wrapping   = TextureWrapping::eRepeat,
			TextureFlags      flags      = TextureFlagBits::eNone
		) noexcept;
		GpuTexture_T()  noexcept = default;
		~GpuTexture_T() noexcept;

		int              getRenderWidth()  const override;
		int              getRenderHeight() const override;
		TextureFormat    getFormat()       const;
		TextureFiltering getFiltering()    const;
		TextureWrapping  getWrapping()     const;
		TextureFlags     getFlags()        const;
		TextureType      getType()         const;

		GLuint glGetHandle();
		
	private:
		GLuint           handle    = 0;
		TextureFormat    format    = TextureFormat::eUnknown;
		TextureFiltering filtering = TextureFiltering::eUnknown;
		TextureWrapping  wrapping  = TextureWrapping::eUnknown;
		TextureFlags     flags     = TextureFlagBits::eNone;
		TextureType      type      = TextureType::eUnknown;
		int              width     = -1;
		int              height    = -1;
	}; 

	class LUNAR_API GpuCubemap_T
	{
	public:
		GpuCubemap_T
		(
			RenderContext_T* context,
			int              width,
			int              height,
			void*            data,
			bool             isSourceHdr
		) noexcept;
		GpuCubemap_T()  noexcept = default;
		~GpuCubemap_T() noexcept = default;

	private:
		GpuTexture environmentMap = nullptr;
		GpuTexture irradianceMap  = nullptr;
		GpuTexture prefilterMap   = nullptr;
		GpuTexture brdfLut        = nullptr;
		int        width          = -1;
		int        height         = -1;
	};
}

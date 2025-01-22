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
		eUnknown            = 0,
		eNearest            = GL_NEAREST,
		eLinear             = GL_LINEAR,
		eLinearMipmapLinear = GL_LINEAR_MIPMAP_LINEAR,
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
		e2DArray = GL_TEXTURE_2D_ARRAY,
		eCubemap = GL_TEXTURE_CUBE_MAP,
	};

	enum class LUNAR_API TextureFlagBits
	{
		eNone     = 0,
		eBindless = 1 << 0
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
			TextureDataFormat dataFormat   = TextureDataFormat::eUnsignedByte,
			TextureFormat     dstFormat    = TextureFormat::eRGBA,
			TextureType       type         = TextureType::e2D,
			TextureFiltering  minFiltering = TextureFiltering::eLinear,
			TextureFiltering  magFiltering = TextureFiltering::eLinear,
			TextureWrapping   wrapping     = TextureWrapping::eRepeat,
			TextureFlags      flags        = TextureFlagBits::eNone
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

		void             generateMipmaps();

		GLuint   glGetHandle();
		GLuint64 glGetBindlessHandle();
		
	public:
		size_t           refCount  = 0;
	private:
		GLuint           handle       = 0;
		GLuint64         bindless     = 0;
		TextureFormat    format       = TextureFormat::eUnknown;
		TextureFiltering minFiltering = TextureFiltering::eUnknown;
		TextureFiltering magFiltering = TextureFiltering::eUnknown;
		TextureWrapping  wrapping     = TextureWrapping::eUnknown;
		TextureFlags     flags        = TextureFlagBits::eNone;
		TextureType      type         = TextureType::eUnknown;
		int              width        = -1;
		int              height       = -1;
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

	public:
		GpuTexture environmentMap = nullptr;
		GpuTexture irradianceMap  = nullptr;
		GpuTexture prefilterMap   = nullptr;
		GpuTexture brdfLut        = nullptr;

	private:
		int        width          = -1;
		int        height         = -1;
	};

	struct LUNAR_API GpuTextureBuilder
	{
	public:
		GpuTextureBuilder()  noexcept = default;
		~GpuTextureBuilder() noexcept = default;

		GpuTextureBuilder& fromByteBuffer
		(
			TextureFormat     srcFormat, 
			TextureDataFormat srcDataFormat, 
			int               width, 
			int               height, 
			void*             data
		); 

		GpuTextureBuilder& destFormat(TextureFormat format);
		GpuTextureBuilder& type(TextureType type);
		GpuTextureBuilder& wrapping(TextureWrapping wrapping);
		GpuTextureBuilder& minFiltering(TextureFiltering filtering);
		GpuTextureBuilder& magFiltering(TextureFiltering filtering);
		GpuTextureBuilder& addFlags(TextureFlags flags);

		GpuTexture build(RenderContext context);
		GpuTexture build(RenderContext_T* context);

	private:
		void*             data             = nullptr;
		int               width            = -1;
		int               height           = -1;
		TextureFormat     srcFormat        = TextureFormat::eUnknown;
		TextureDataFormat srcDataFormat    = TextureDataFormat::eUnsignedByte;
		TextureFormat     dstFormat        = TextureFormat::eUnknown;
		TextureType       textureType      = TextureType::eUnknown;
		TextureFiltering  minFilter        = TextureFiltering::eUnknown;
		TextureFiltering  magFilter        = TextureFiltering::eUnknown;
		TextureWrapping   textureWrapping  = TextureWrapping::eUnknown;
		TextureFlags      flags            = TextureFlagBits::eNone;
	};
}

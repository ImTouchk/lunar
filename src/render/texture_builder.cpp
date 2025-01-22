#include <lunar/render.hpp>
#include <lunar/debug.hpp>

namespace lunar::Render
{
	GpuTextureBuilder& GpuTextureBuilder::fromByteBuffer
	(
		TextureFormat     srcFormat,
		TextureDataFormat srcDataFormat,
		int               width,
		int               height,
		void*             data
	)
	{
		this->srcFormat     = srcFormat;
		this->srcDataFormat = srcDataFormat;
		this->width         = width;
		this->height        = height;
		this->data          = data;
		return *this;
	}

	GpuTextureBuilder& GpuTextureBuilder::destFormat(TextureFormat format)
	{
		this->dstFormat = format;
		return *this;
	}

	GpuTextureBuilder& GpuTextureBuilder::type(TextureType type)
	{
		this->textureType = type;
		return *this;
	}

	GpuTextureBuilder& GpuTextureBuilder::wrapping(TextureWrapping wrapping)
	{
		this->textureWrapping = wrapping;
		return *this;
	}

	GpuTextureBuilder& GpuTextureBuilder::minFiltering(TextureFiltering filtering)
	{
		this->minFilter = filtering;
		return *this;
	}

	GpuTextureBuilder& GpuTextureBuilder::magFiltering(TextureFiltering filtering)
	{
		this->magFilter = filtering;
		return *this;
	}

	GpuTextureBuilder& GpuTextureBuilder::addFlags(TextureFlags flags)
	{
		this->flags = this->flags | flags;
		return *this;
	}

	GpuTexture GpuTextureBuilder::build(RenderContext context)
	{
		return build(context.get());
	}

	GpuTexture GpuTextureBuilder::build(RenderContext_T* context)
	{
		return context->createTexture
		(
			this->width,
			this->height,
			this->data,
			this->srcFormat,
			this->srcDataFormat,
			this->dstFormat,
			this->textureType,
			this->minFilter,
			this->magFilter,
			this->textureWrapping,
			this->flags
		);
	}
}
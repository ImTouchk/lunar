#include <lunar/render/texture.hpp>
#include "../../stb_image.h"

namespace Render
{
	GLuint Texture::glGetHandle()
	{
		return _glHandle;
	}

	TextureBuilder& TextureBuilder::setSize(int width, int height)
	{
		this->width = width;
		this->height = height;
		return *this;
	}

	TextureBuilder& TextureBuilder::setDstFormat(TextureFormat format)
	{
		this->dstFormat = format;
		return *this;
	}

	TextureBuilder& TextureBuilder::setByteFormat(TextureByteFormat format)
	{
		this->byteFormat = format;
		return *this;
	}

	TextureBuilder& TextureBuilder::setWrapping(TextureWrapping wrapping)
	{
		this->wrapping = wrapping;
		return *this;
	}

	TextureBuilder& TextureBuilder::setFiltering(TextureFiltering filtering)
	{
		this->filtering = filtering;
		return *this;
	}

	TextureBuilder& TextureBuilder::fromImagePath(const Fs::Path& path)
	{
		stbi_set_flip_vertically_on_load(true);

		this->path = path;
		this->rawBytes = reinterpret_cast<void*>(
			stbi_load(path.generic_string().c_str(), &width, &height, &channels, 0)
		);
		return *this;
	}

	TextureBuilder& TextureBuilder::fromByteArray(TextureFormat format, int width, int height, void* data)
	{
		this->path = "";
		this->srcFormat = format;
		this->width = width;
		this->height = height;
		this->rawBytes = data;
		return *this;
	}

	TextureBuilder& TextureBuilder::useRenderContext(std::shared_ptr<RenderContext>& context)
	{
		this->context = context;
		return *this;
	}

	TextureBuilder& TextureBuilder::build()
	{
#ifdef LUNAR_OPENGL
		_glBuild();
#endif
		if(not path.empty())
			stbi_image_free(rawBytes);

		return *this;
	}

	Texture TextureBuilder::getResult()
	{
		return std::move(result);
	}

	CubemapBuilder& CubemapBuilder::useRenderContext(std::shared_ptr<RenderContext>& context)
	{
		this->context = context;
		return *this;
	}

	CubemapBuilder& CubemapBuilder::fromHDRFile(const Fs::Path& path)
	{
		stbi_set_flip_vertically_on_load(true);

		rawBytes[0] = reinterpret_cast<void*>(
			stbi_loadf(path.generic_string().c_str(), &width[0], &height[0], &channels[0], 0)
		);

		isHdr = true;
		files = { path };

		return *this;
	}

	CubemapBuilder& CubemapBuilder::fromFiles(const std::initializer_list<Fs::Path>& files)
	{
		stbi_set_flip_vertically_on_load(true);

		this->files = files;
		for (size_t i = 0; i < files.size(); i++)
		{
			rawBytes[i] = reinterpret_cast<void*>(
				stbi_load(this->files[i].generic_string().c_str(), &width[i], &height[i], &channels[i], 0)
			);
		}

		isHdr = false;

		return *this;
	}

	CubemapBuilder& CubemapBuilder::build()
	{
#ifdef LUNAR_OPENGL
		_glBuild();
#endif

		for (size_t i = 0; i < files.size(); i++)
			stbi_image_free(rawBytes[i]);

		return *this;
	}

	Cubemap CubemapBuilder::getResult()
	{
		return std::move(result);
	}
}

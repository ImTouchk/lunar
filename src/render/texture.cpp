#include <lunar/render/texture.hpp>
#include "../../stb_image.h"

namespace Render
{
	TextureBuilder& TextureBuilder::setFiltering(TextureFiltering filtering)
	{
		this->filtering = filtering;
		return *this;
	}

	TextureBuilder& TextureBuilder::fromImagePath(const Fs::Path& path)
	{
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
}

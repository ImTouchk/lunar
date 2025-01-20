#include <lunar/file/image_file.hpp>
#include <fstream>
#include <sstream>

#include "../../stb_image.h"


namespace Fs
{
	bool ImageFile::fromFile(const Path& path)
	{
		if (!fileExists(path))
			return false;

		stbi_set_flip_vertically_on_load(true);

		bytes = reinterpret_cast<void*>(
			stbi_loadf(path.generic_string().c_str(), &width, &height, &channels, 0)
		);
		return true;
	}

	void ImageFile::toFile(const Path& path)
	{
		throw;
	}
}

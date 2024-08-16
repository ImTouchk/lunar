#include <file/filesystem.hpp>

namespace Fs
{
	Path baseDirectory()
	{
		return std::filesystem::current_path();
	}

	Path dataDirectory()
	{
		return baseDirectory()
				.append("data");
	}

	bool fileExists(const Path& path)
	{
		return std::filesystem::exists(path) && std::filesystem::is_regular_file(path);
	}

	void Resource::toFile(const Path& path)
	{
	}

	bool Resource::fromFile(const Path& path)
	{
		return false;
	}
}

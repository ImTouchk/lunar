#include <lunar/file/filesystem.hpp>

namespace Fs
{
	Path fromData(const std::string_view& path)
	{
		return dataDirectory().append(path);
	}

	Path fromBase(const std::string_view& base)
	{
		return baseDirectory().append(base);
	}

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

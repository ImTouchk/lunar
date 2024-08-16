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
}

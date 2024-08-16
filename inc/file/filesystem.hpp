#pragma once
#include <filesystem>

namespace Fs
{
	using Path = std::filesystem::path;

	Path baseDirectory();
	Path dataDirectory();

	class Resource
	{
	public:
		virtual void toFile(const Path& path);
		virtual bool fromFile(const Path& path);
	};
}

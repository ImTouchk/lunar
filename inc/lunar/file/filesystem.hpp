#pragma once
#include <lunar/api.hpp>
#include <filesystem>
#include <string_view>

namespace Fs
{
	using Path = std::filesystem::path;

	LUNAR_API Path baseDirectory();
	LUNAR_API Path dataDirectory();
	
	LUNAR_API Path fromData(const std::string_view& path);
	LUNAR_API Path fromBase(const std::string_view& path);

	LUNAR_API bool fileExists(const Path& path);

	class LUNAR_API Resource
	{
	public:
		virtual void toFile(const Path& path);
		virtual bool fromFile(const Path& path);
	};
}

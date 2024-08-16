#pragma once
#include <file/filesystem.hpp>
#include <string>

namespace Fs
{
	class TextFile : public Resource
	{
	public:
		TextFile(const Path& path) { fromFile(path); }
		TextFile() = default;

		void toFile(const Path& path) override;
		bool fromFile(const Path& path) override;

		std::string content;
	};
}

#pragma once
#include <file/filesystem.hpp>
#include <nlohmann/json.hpp>

namespace Fs
{
	class JsonFile : public Resource
	{
	public:
		bool fromFile(const Path& path) override;
		void toFile(const Path& path) override;

		nlohmann::json content;
	};
}

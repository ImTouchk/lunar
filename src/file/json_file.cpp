#include <file/json_file.hpp>
#include <fstream>

namespace Fs
{
	bool JsonFile::fromFile(const Path& path)
	{
		if (!fileExists(path))
			return false;

		auto res_file = std::ifstream(path);
		content = nlohmann::json::parse(res_file);
		res_file.close();
		return true;
	}

	void JsonFile::toFile(const Path& path)
	{
		auto res_file = std::ofstream(path);
		res_file << content.dump();
		res_file.close();
	}
}

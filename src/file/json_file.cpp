#include <lunar/file/json_file.hpp>
#include <fstream>

namespace Fs
{
	JsonFile::JsonFile(const Path& path)
	{
		fromFile(path);
	}

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

	bool JsonObject::fromFile(const Path& path)
	{
		if (!fileExists(path))
			return false;

		auto res_file = std::ifstream(path);
		auto res_data = nlohmann::json::parse(res_file);
		fromJson(res_data);
		res_file.close();
		return true;

		return true;
	}

	void JsonObject::toFile(const Path& path)
	{
		auto res_file = std::ofstream(path);
		res_file << toJson().dump();
		res_file.close();
	}

	void JsonObject::fromJson(nlohmann::json& json)
	{
		throw;
	}

	nlohmann::json Fs::JsonObject::toJson()
	{
		throw;
		return {};
	}
}

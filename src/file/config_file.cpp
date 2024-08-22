#include <lunar/file/config_file.hpp>
#include <lunar/utils/lexer.hpp>
#include <lunar/debug/log.hpp>
#include <sstream>
#include <fstream>
#include <string>

namespace Fs
{
	ConfigFile::ConfigFile(const Path& path)
	{
		fromFile(path);
	}

	bool ConfigFile::fromFile(const Path& path)
	{
		if (!fileExists(path))
			return false;

		auto res_file = std::ifstream(path);
		auto res_buf = std::stringstream();
		res_buf << res_file.rdbuf();
		
		auto file_content = res_buf.str();
		auto lexer = Utils::Lexer(file_content);
		std::string key, value;
		while (!lexer.atEnd())
		{
			if (lexer.consume('#'))
			{
				lexer.skipLine();
			}
			if (lexer.consumeTemplate("{:s} = {:s}", &key, &value))
			{
				content.insert(std::pair<std::string, std::string>(key, value));
			}
		}

		return true;
	}

	void ConfigFile::toFile(const Path& path)
	{
		auto res_file = std::ofstream(path);
		for (auto& [key, value] : content) {
			res_file << std::format("{} = {}\n", key, value);
		}
	}
}

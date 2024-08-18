#include <lunar/file/text_file.hpp>
#include <fstream>
#include <sstream>

namespace Fs
{
	bool TextFile::fromFile(const Path& path)
	{
		if (!fileExists(path))
			return false;

		auto res_file = std::ifstream(path);
		auto res_buf = std::stringstream();
		res_buf << res_file.rdbuf();
		content = res_buf.str();
		res_file.close();
		return true;
	}

	void TextFile::toFile(const Path& path)
	{
		auto res_file = std::ofstream(path);
		res_file << content;
		res_file.close();
	}
}

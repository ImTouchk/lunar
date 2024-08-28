#include <lunar/file/binary_file.hpp>
#include <lunar/debug.hpp>
#include <fstream>
#include <sstream>

namespace Fs
{
	bool BinaryFile::fromFile(const Path& path)
	{
		if (!fileExists(path))
			return false;

		// TODO: error handling

		auto res_file = std::ifstream(path, std::ios::ate | std::ios::binary);
		size_t file_size = (size_t)res_file.tellg();
		content = std::vector<char>(file_size);
		res_file.seekg(0);
		res_file.read(content.data(), file_size);
		res_file.close();
		return true;
	}

	void BinaryFile::toFile(const Path& path)
	{
		// TOOD: implement
		DEBUG_NOT_IMPLEMENTED();
	}
}

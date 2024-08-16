#pragma once
#include <vector>
#include <string>
#include <string_view>

namespace Utils
{
	class ArgumentParser
	{
	public:
		ArgumentParser(int argc, char* argv[]);

		int has(const std::string_view& value) const;
		bool hasAt(int index) const;
		const std::string& get(int index) const;

	private:
		std::vector<std::string> arguments;
	};
}

#pragma once
#include <lunar/api.hpp>
#include <string_view>
#include <string>
#include <vector>

namespace Utils
{
	class LUNAR_API ArgumentParser
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

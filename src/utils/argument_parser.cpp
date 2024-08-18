#include <lunar/utils/argument_parser.hpp>

namespace Utils
{
	ArgumentParser::ArgumentParser(int argc, char* argv[])
		: arguments()
	{
		for (int i = 0; i < argc; i++)
			arguments.push_back(std::string(argv[i]));
	}

	int ArgumentParser::has(const std::string_view& value) const
	{
		for (int i = 0; i < arguments.size(); i++)
			if (arguments[i].compare(value) == 0)
				return i;

		return -1;
	}

	bool ArgumentParser::hasAt(int index) const
	{
		return index < arguments.size();
	}

	const std::string& ArgumentParser::get(int index) const
	{
		return arguments.at(index);
	}
}

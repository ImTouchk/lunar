#pragma once
#include <file/filesystem.hpp>
#include <string>
#include <utils/lexer.hpp>
#include <string_view>
#include <map>

namespace UI
{
	enum class SheetType
	{
		Uninitialized,
		Class,
		Identifier
	};

	class Stylesheet
	{
	public:
		Stylesheet(SheetType type);
		
		void add(const std::string& name, const std::string& value);
		std::string& get(const std::string& name);

	private:
		SheetType type;
		std::map<size_t, std::string> registry;
	};

	class StylesheetDictionary
	{
	public:
		StylesheetDictionary(const Fs::Path& path);

		void parseFile(const Fs::Path& path);

		Stylesheet& get(const std::string& name);
	private:
		void add(const std::string& name, SheetType type);
		void parseSheet(Utils::Lexer& lexer, Stylesheet& sheet);

		std::map<size_t, Stylesheet> dictionary;
	};
}

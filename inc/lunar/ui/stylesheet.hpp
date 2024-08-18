#pragma once
#include <lunar/file/filesystem.hpp>
#include <lunar/utils/lexer.hpp>
#include <lunar/api.hpp>
#include <string_view>
#include <string>
#include <map>

namespace UI
{
	enum class LUNAR_API SheetType
	{
		Uninitialized,
		Class,
		Identifier
	};

	class LUNAR_API Stylesheet
	{
	public:
		Stylesheet(SheetType type);
		
		void add(const std::string& name, const std::string& value);
		std::string& get(const std::string& name);

	private:
		SheetType type;
		std::map<size_t, std::string> registry;
	};

	class LUNAR_API StylesheetDictionary
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

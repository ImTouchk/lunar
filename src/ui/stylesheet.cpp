#include <utils/lexer.hpp>
#include <ui/stylesheet.hpp>
#include <file/text_file.hpp>
#include <debug/log.hpp>

namespace UI
{
	Stylesheet::Stylesheet(SheetType type)
		: type(type)
	{
	}

	void Stylesheet::add(const std::string& name, const std::string& value)
	{
		registry.insert(std::pair<size_t, std::string>(std::hash<std::string>{}(name), value));
	}

	std::string& Stylesheet::get(const std::string& name)
	{
		return registry.at(std::hash<std::string>{}(name));
	}

	StylesheetDictionary::StylesheetDictionary(const Fs::Path& path)
		: dictionary()
	{
		parseFile(path);
	}

	void StylesheetDictionary::add(const std::string& name, SheetType type)
	{
		dictionary.insert(std::pair<size_t, Stylesheet>(std::hash<std::string>{}(name), Stylesheet(type)));
	}

	Stylesheet& StylesheetDictionary::get(const std::string& name)
	{
		return dictionary.at(std::hash<std::string>{}(name));
	}

	void StylesheetDictionary::parseSheet(Utils::Lexer& lexer, Stylesheet& sheet)
	{
		int error_tries = 0;

		while (!lexer.consume('}'))
		{
			std::string key, value;
			if (!lexer.consumeTemplate(" {:s_until}: {:s_until}; ", ':', &key, ';', &value))
			{
				DEBUG_ERROR("Unexpected token inside css file.");
				DEBUG_ERROR("Error message: {}", lexer.getLastError());
				error_tries++;

				if (error_tries >= 3)
				{
					DEBUG_ERROR("Too many attempts. Abandoning...");
					return;
				}
			}
			else
			{
				sheet.add(key, value);
				DEBUG_LOG("-- {}: {}", key, value);
			}
		}
	}

	void StylesheetDictionary::parseFile(const Fs::Path& path)
	{
		auto file = Fs::TextFile(path);
		auto lexer = Utils::Lexer(file.content);
		auto identifier = std::string();
		char sheet_type;
		while (lexer.consumeTemplate(" {:c}{:s_id} { ", &sheet_type, &identifier))
		{
			switch (sheet_type)
			{
			case '.': add(identifier, SheetType::Class); break;
			case '#': add(identifier, SheetType::Identifier); break;
			default: add(identifier, SheetType::Uninitialized); break;
			}

			DEBUG_LOG("Reading CSS properties of \"{}{}\".", sheet_type, identifier);
			parseSheet(lexer, get(identifier));
		}
	}
}

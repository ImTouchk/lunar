#pragma once
#include <lunar/api.hpp>
#include <lunar/file/text_file.hpp>
#include <string_view>
#include <string>

namespace Utils::Exp
{
	class LUNAR_API Lexer
	{
	public:
		Lexer(std::string&& source);
		~Lexer() = default;

		Lexer& skipWhitespaces();
		[[nodiscard]] std::optional<float> parseFloat();
		[[nodiscard]] std::optional<int> parseInt();
		[[nodiscard]] std::optional<bool> parseBoolean();
		[[nodiscard]] std::string_view parseString();
		[[nodiscard]] bool parse(const char* string, ...);
		[[nodiscard]] bool parseLine(const char* string, ...);
		
	private:
		[[nodiscard]] bool vparse(const char* string, va_list args);

		std::string source;
		std::string::iterator pointer;
		friend struct LexerBuilder;
	};

	struct LUNAR_API LexerBuilder
	{
		LexerBuilder() = default;
		~LexerBuilder() = default;

		LexerBuilder& appendTextFile(const Fs::Path& path);
		LexerBuilder& appendString(const std::string& string);
		[[nodiscard]] Lexer create();
	private:
		std::string source = "";
	};
}
#pragma once
#include <string>

namespace Utils
{
	class Lexer
	{
	public:
		explicit Lexer(const char* text);
		explicit Lexer(const std::string_view& text);
		
		bool atEnd();
		void advance();
		char current();
		char next();

		void skipLine();
		void skipWhitespace();

		bool consume(char c);
		bool consume(const std::string_view& sequence);
		bool consumeTemplate(const char* lexerTemplate, ...);

		int number();
		float real();
		std::string string(char stopChar = ' ');
		std::string identifier();

		const std::string& getLastError() const;

		// Go to last token before error was thrown
		void unwind();

	private:
		std::string text;
		std::string::const_iterator it;
		std::string::const_iterator lastBeforeError;
		std::string lastError;
	};
}

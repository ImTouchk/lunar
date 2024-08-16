#include <utils/lexer.hpp>
#include <charconv>
#include <cstdarg>

namespace Utils
{
	Lexer::Lexer(const char* text)
		: text(text),
		it(this->text.begin())
	{
	}

	Lexer::Lexer(const std::string_view& text)
		: text(text),
		it(this->text.begin())
	{
	}

	bool Lexer::atEnd()
	{
		return it >= text.end();
	}

	void Lexer::advance()
	{
		it++;
	}

	char Lexer::current()
	{
		return *it;
	}

	char Lexer::next()
	{
		if (it + 1 >= text.end())
			return '\0';

		return *(it + 1);
	}

	void Lexer::skipWhitespace()
	{
		while (!atEnd() && isspace(current()))
			advance();
	}

	void Lexer::skipLine()
	{
		while (!atEnd() && current() != '\n')
			advance();
	}

	float Lexer::real()
	{
		auto start = it;
		if (current() == '-')
			advance();

		while (!atEnd() && isdigit(current()))
		{
			advance();
		}

		if (current() == '.' && isdigit(next()))
		{
			while (!atEnd() && isdigit(current()))
				advance();
		}

		float value;
		auto result = std::from_chars(start._Ptr, it._Ptr, value);
		if (result.ec != std::errc())
			return (float)0xCACA;
	}

	int Lexer::number()
	{
		auto start = it;
		if (current() == '-')
			advance();

		while (!atEnd() && isdigit(current()))
		{
			advance();
		}

		int value;
		auto result = std::from_chars(start._Ptr, it._Ptr, value);
		if (result.ec != std::errc())
			return 0xCACA;

		return value;
	}

	std::string Utils::Lexer::string()
	{
		auto start = it;
		size_t count = 0;

		while (!atEnd() && !isspace(current()))
		{
			count++;
			advance();
		}

		auto end = it;
		return std::string(start, end);
	}

	bool Lexer::consume(char c)
	{
		if (current() == c)
		{
			advance();
			return true;
		}
		else
			return false;
	}

	bool Lexer::consume(const std::string_view& sequence)
	{
		size_t i = 0;
		auto start = it;
		while (!atEnd() && i < sequence.size())
		{
			if (current() != sequence[i])
				break;

			advance();
			i++;
		}

		if (i == sequence.size())
			return true;
		else
		{
			it = start;
			return false;
		}
	}

	bool Lexer::consumeTemplate(const char* lexerTemplate, ...)
	{
		va_list args;
		va_start(args, lexerTemplate);

		auto sub_lexer = Lexer(lexerTemplate);
		while (!sub_lexer.atEnd())
		{
			if (sub_lexer.consume("{:d}"))
			{
				int* number_ptr = va_arg(args, int*);
				*number_ptr = number();
			}
			else if (sub_lexer.consume("{:f}"))
			{
				float* number_ptr = va_arg(args, float*);
				*number_ptr = real();
			}
			else if (sub_lexer.consume("{:s}"))
			{
				std::string* str_ptr = va_arg(args, std::string*);
				*str_ptr = string();
			}
			else
			{
				if (sub_lexer.current() != current())
					return false;

				if (sub_lexer.current() == ' ')
					skipWhitespace();
				else
					advance();

				sub_lexer.advance();
			}
		}

		return true;
	}
}

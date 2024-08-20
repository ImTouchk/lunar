#include <lunar/utils/lexer.hpp>
#include <lunar/debug/log.hpp>
#include <charconv>
#include <cstdarg>
#include <format>

namespace Utils
{
	Lexer::Lexer(const char* text)
		: text(text),
		it(this->text.begin()),
		lastError(),
		lastBeforeError(it)
	{
	}

	Lexer::Lexer(const std::string_view& text)
		: text(text),
		it(this->text.begin()),
		lastError(),
        lastBeforeError(it)
	{
	}

	const std::string& Lexer::getLastError() const
	{
		return lastError;
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
		auto result = std::from_chars(&(*start), &(*it), value);
        if (result.ec != std::errc()) {
            return (float)0xCACA;
        }
        else {
            return value;
        }
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
		auto result = std::from_chars(&(*start), &(*it), value);
		if (result.ec != std::errc())
			return 0xCACA;

		return value;
	}

	std::string Lexer::string(char stopChar)
	{
		auto start = it;
		size_t count = 0;

		while (!atEnd())
		{
			if (stopChar == ' ')
			{
				if (!isspace(current()))
					break;
			}
			else
			{
				if (current() == stopChar)
					break;
			}

			count++;
			advance();
		}

		auto end = it;
		return std::string(start, end);
	}

	std::string Lexer::identifier()
	{
		auto start = it;
		size_t count = 0;

		while (!atEnd() && 
			(isalpha(current()) || 
				( count > 0 && 
					(isdigit(current()) || current() == '-') 
				)
			)
		)
		{
			count++;
			advance();
		}

		auto end = it;
		return std::string(start, end);
	}

	void Lexer::unwind()
	{
		it = lastBeforeError;
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

		auto start = it;
		auto sub_lexer = Lexer(lexerTemplate);
		while (!sub_lexer.atEnd())
		{
			if (atEnd())
			{
				lastError = "Reached end of string.";
				lastBeforeError = start;
				return false;
			}

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
			else if (sub_lexer.consume("{:s_id}"))
			{
				std::string* str_ptr = va_arg(args, std::string*);
				*str_ptr = identifier();
			}
			else if (sub_lexer.consume("{:s_until}"))
			{
				char c = va_arg(args, int);
				std::string* str_ptr = va_arg(args, std::string*);
				*str_ptr = string(c);
			}
			else if (sub_lexer.consume("{:c}"))
			{
				char* char_ptr = va_arg(args, char*);
				*char_ptr = current();
				advance();
			}
			else
			{
				if (sub_lexer.current() == ' ')
				{
					skipWhitespace();
					sub_lexer.advance();
					continue;
				}

				if (sub_lexer.current() != current())
				{
					lastError = std::format(
						"Expected token '{}' from lexer template \"{}\". Found '{}' instead.",
						printable(sub_lexer.current()), printable(lexerTemplate),
						printable(current())
					);
					lastBeforeError = start;
					return false;
				}

				advance();
				sub_lexer.advance();
			}
		}

		return true;
	}
}

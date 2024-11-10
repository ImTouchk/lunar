#include <lunar/exp/utils/lexer.hpp>
#include <lunar/debug.hpp>
#include <cstdarg>

namespace Utils::Exp
{
	LexerBuilder& LexerBuilder::appendTextFile(const Fs::Path& path)
	{
		auto file = Fs::TextFile(path);
		source.append(file.content);

		return *this;
	}

	LexerBuilder& LexerBuilder::appendString(const std::string& string)
	{
		source.append(string);
		return *this;
	}

	LexerBuilder& LexerBuilder::enableErrorPrinting()
	{
		flags = flags | LexerFlagBits::ePrintErrors;
		return *this;
	}

	Lexer LexerBuilder::create()
	{
		return Lexer(std::move(source), flags);
	}

	Lexer::Lexer(std::string&& source, LexerFlagBits flags)
		: source(source),
		pointer(this->source.begin()),
		flags(flags),
		lastError(),
		line(1)
	{
	}

	size_t Lexer::getCursorLine() const
	{
		return line;
	}

	std::string_view Lexer::getSource() const
	{
		return source;
	}

	void Lexer::parseError(const std::string& error)
	{
		lastError = std::format("Parse error at source line {}: {}\n", line, error);
		if (flags & LexerFlagBits::ePrintErrors)
		{
			DEBUG_WARN("{}", lastError);
		}
	}

	bool Lexer::isCursorAtEnd() const
	{
		return pointer == source.end();
	}

	std::string::iterator Lexer::getCursor() const
	{
		return pointer;
	}

	Lexer& Lexer::setCursorPos(std::string::iterator pos)
	{
		pointer = pos;
		return *this;
	}

	Lexer& Lexer::skipWhitespaces()
	{
		while (pointer != source.end() && isspace(*pointer))
		{
			if (*pointer == '\n')
				line++;

			pointer++;
		}

		return *this;
	}

	std::string_view Lexer::parseIdentifier()
	{
		skipWhitespaces();

		auto start = pointer;

		while (
			pointer != source.end() &&
			(
				isalpha(*pointer) || 
				*pointer == '_' || 
				(pointer != start && (
					isalnum(*pointer) ||
					*pointer == '-' ||
					*pointer == '.'
				))
			)
		)
		{
			pointer++;
		}

		return std::string_view(start, pointer);
	}

	std::string_view Lexer::parseString(char delimiter)
	{
		skipWhitespaces();

		auto start = pointer;

		while (
			pointer != source.end() && 
			(
				(delimiter == ' ' && !isspace(*pointer)) ||
				*pointer != delimiter
			)
		)
		{
			pointer++;
		}

		return std::string_view(start, pointer);
	}

	std::optional<bool> Lexer::parseBoolean()
	{
		if (parse("true"))
			return true;
		else if (parse("false"))
			return false;
		else 
			return parseInt();
	}

	std::optional<int> Lexer::parseInt()
	{
		auto start = pointer;
		
		if (*pointer == '-')
			pointer++;

		while (pointer != source.end() && isdigit(*pointer))
			pointer++;

		int value;
		auto result = std::from_chars(&(*start), &(*pointer), value);
		if (result.ec != std::errc())
		{
			if (start == pointer)
				parseError(std::format("Expected integer, found token '{}' instead", *pointer));
			else if (result.ec == std::errc::invalid_argument)
				parseError(std::format("Expected integer, found '{}' instead.", std::string_view(start, pointer)));
			else if (result.ec == std::errc::result_out_of_range)
				parseError(std::format("Integer '{}' out of range.", std::string_view(start, pointer)));

			pointer = start;
			return std::nullopt;
		}
		else
			return value;
	}

	std::optional<float> Lexer::parseFloat()
	{
		auto start = pointer;

		if (*pointer == '-')
			pointer++;

		while (pointer != source.end() && isdigit(*pointer))
			pointer++;

		if (*pointer == '.')
		{
			pointer++;
			while (pointer != source.end() && isdigit(*pointer))
				pointer++;
		}

		float value;
		auto result = std::from_chars(&(*start), &(*pointer), value);
		if (result.ec != std::errc())
		{
			if (start == pointer)
				parseError(std::format("Expected floating point number, found token '{}' instead", *pointer));
			else if (result.ec == std::errc::invalid_argument)
				parseError(std::format("Expected floating point number, found '{}' instead.", std::string_view(start, pointer)));
			else if (result.ec == std::errc::result_out_of_range)
				parseError(std::format("Floating point number '{}' out of range.", std::string_view(start, pointer)));

			pointer = start;
			return std::nullopt;
		}
		else
			return value;
	}

	bool Lexer::parseFromDictionary(const Lexer::TokenDictionary& dictionary)
	{
		for (auto& token : dictionary)
		{
			auto start = pointer;

			if (!parse(token.name))
				continue;

			if (
				pointer != source.end() &&
				(isalnum(*pointer) && !isspace(*pointer)) // this is the biggest piece of shit i've ever written and i'm not proud of it
			)
			{
				pointer = start;
				continue;
			}
			
			token.resolver();
			return true;
		}

		return false;
	}

	bool Lexer::parse(const char* string, ...)
	{
		bool res;
		va_list args;
		va_start(args, string);
		res = vparse(string, args);
		va_end(args);
		return res;
	}

	bool Lexer::parseLine(const char* string, ...)
	{
		auto start = pointer;

		bool res;
		va_list args;
		va_start(args, string);
		
		skipWhitespaces();
		res = vparse(string, args);
		skipWhitespaces();

		va_end(args);

		if (!res)
			pointer = start;

		return res;
	}

	bool Lexer::vparse(const char* string, va_list args)
	{
		auto start = pointer;
		bool error_flag = false;

		auto view = std::string_view(string);

		for (auto it = view.begin(); it != view.end(); it++)
		{
			if (error_flag)
				break;

			if (pointer == source.end())
			{
				parseError(std::format("End of string, expected token '{}' instead", *it));
				error_flag = true;
				break;
			}

			auto next = it + 1;

			if (*it != '{' || next == view.end() || *next != ':')
			{
				if (*it == ' ')
				{
					skipWhitespaces();
				}
				else if (*pointer != *it)
				{
					parseError(std::format("Expected token '{}', found '{}' instead", *it, *pointer));
					error_flag = true;
					break;
				}
				else
					pointer++;
			}
			else
			{
				auto template_end = it;
				while (template_end != view.end() && *template_end != '}')
					template_end++;

				if (template_end == view.end())
				{
					DEBUG_ASSERT(false); // invalid template string
					error_flag = true;
					break;
				}

				auto template_type = std::string_view(it, template_end + 1);
				it = template_end;

				if (template_type.compare("{:d}") == 0)
				{
					int* number_ptr = va_arg(args, int*);
					auto res = parseInt();

					if (!res.has_value())
					{
						error_flag = true;
						break;
					}

					*number_ptr = res.value();
				}
				else if (template_type.compare("{:f}") == 0)
				{
					float* number_ptr = va_arg(args, float*);
					auto res = parseFloat();

					if (!res.has_value())
					{
						error_flag = true;
						break;
					}

					*number_ptr = res.value();
				}
				else if (template_type.compare("{:s}") == 0)
				{
					std::string_view* string_ptr = va_arg(args, std::string_view*);
					*string_ptr = parseString();
				}
				else if (template_type.compare("{:s_id}") == 0)
				{
					std::string_view* string_ptr = va_arg(args, std::string_view*);
					*string_ptr = parseIdentifier();
				}
				else if (template_type.compare("{:s_delim}") == 0)
				{
					char delimiter = va_arg(args, char);
					std::string_view* string_ptr = va_arg(args, std::string_view*);
					*string_ptr = parseString(delimiter);
				}
				else if (template_type.compare("{:b}") == 0)
				{
					bool* bool_ptr = va_arg(args, bool*);
					auto res = parseBoolean();

					if (!res.has_value())
					{
						error_flag = true;
						break;
					}

					*bool_ptr = res.value();
				}
				else
				{
					DEBUG_ASSERT(false); // invalid template string
					throw;
				}
			}
		}

		if (error_flag)
			pointer = start;

		return !error_flag;
	}
}

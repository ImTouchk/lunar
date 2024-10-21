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

	Lexer LexerBuilder::create()
	{
		return Lexer(std::move(source));
	}

	Lexer::Lexer(std::string&& source)
		: source(source),
		pointer(this->source.begin())
	{
	}

	Lexer& Lexer::skipWhitespaces()
	{
		while (pointer != source.end() && isspace(*pointer))
			pointer++;

		return *this;
	}

	std::string_view Lexer::parseString()
	{
		skipWhitespaces();

		auto start = pointer;

		while (pointer != source.end() && !isspace(*pointer))
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
			pointer = start;
			return std::nullopt;
		}
		else
			return value;
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
				DEBUG_ERROR("End of string");
				error_flag = true;
				break;
			}

			auto next = it + 1;

			if (*it != '{' || next == view.end() || *next != ':')
			{
				if (*it == ' ')
					skipWhitespaces();
				else if (*pointer != *it)
				{
					DEBUG_ERROR("Expected '{}', found '{}' instead", *it, *pointer);
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
					DEBUG_ERROR("Invalid parse template string");
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
						DEBUG_ERROR("Failed to parse integer");
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
						DEBUG_ERROR("Failed to parse floating point number");
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
				else if (template_type.compare("{:b}") == 0)
				{
					bool* bool_ptr = va_arg(args, bool*);
					auto res = parseBoolean();

					if (!res.has_value())
					{
						DEBUG_ERROR("Failed to parse boolean value");
						error_flag = true;
						break;
					}

					*bool_ptr = res.value();
				}
				else
				{
					DEBUG_ERROR("Invalid parse template string");
					throw;
				}
			}
		}

		if (error_flag)
			pointer = start;

		return !error_flag;
	}
}

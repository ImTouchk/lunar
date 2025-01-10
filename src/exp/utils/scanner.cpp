#include <lunar/exp/utils/scanner.hpp>

namespace Utils::Exp
{
	Scanner::Scanner(const std::string& source)
		: source(source)
	{
	}

	Scanner::Scanner(const std::string_view& source)
		: source(source)
	{
	}

	Scanner& Scanner::run()
	{
		while (!isCursorAtEnd())
		{
			start = cursor;
			next();
		}

		tokens.emplace_back(TokenType::eEof, currentLine);
		return *this;
	}

	const std::vector<Token>& Scanner::getResult() const
	{
		return tokens;
	}
	
	char Scanner::advance()
	{
		char c = *cursor;
		cursor++;
		return c;
	}

	void Scanner::next()
	{
		char c    = advance();
		auto view = makeView();
		switch (c)
		{
		case '{': tokens.emplace_back(TokenType::eLeftBrace,  currentLine, view); break;
		case '}': tokens.emplace_back(TokenType::eRightBrace, currentLine, view); break;
		case '(': tokens.emplace_back(TokenType::eLeftParen,  currentLine, view); break;
		case ')': tokens.emplace_back(TokenType::eRightParen, currentLine, view); break;
		case ',': tokens.emplace_back(TokenType::eComma,      currentLine, view); break;
		case '.': tokens.emplace_back(TokenType::eDot,        currentLine, view); break;
		case '-': tokens.emplace_back(TokenType::eMinus,      currentLine, view); break;
		case '+': tokens.emplace_back(TokenType::ePlus,       currentLine, view); break;
		case ';': tokens.emplace_back(TokenType::eSemicolon,  currentLine, view); break;
		case ':': tokens.emplace_back(TokenType::eDoubleColon,currentLine, view); break;
		case '*': tokens.emplace_back(TokenType::eStar,       currentLine, view); break;
		case '/': tokens.emplace_back(TokenType::eSlash,      currentLine, view); break;
		case '!': tokens.emplace_back(match('=') ? TokenType::eBangEqual    : TokenType::eBang,    currentLine, view); break;
		case '=': tokens.emplace_back(match('=') ? TokenType::eEqualEqual   : TokenType::eEqual,   currentLine, view); break;
		case '<': tokens.emplace_back(match('=') ? TokenType::eLessEqual    : TokenType::eLess,    currentLine, view); break;
		case '>': tokens.emplace_back(match('=') ? TokenType::eGreaterEqual : TokenType::eGreater, currentLine, view); break;
		case '\n': currentLine++; break;
		case '\r':
		case '\t':
		case '\0':
		case ' ':
			if (flags & ScannerFlags::eScanWhitespaces)
				tokens.emplace_back(TokenType::eWhitespace, currentLine, view);

			break;
		default:
		{
			if (isdigit(c))
				number();
			else if (isalpha(c) || c == '_')
				identifier();
			else
			{
				DEBUG_ERROR("Unexpected character '{}'.", c);
				throw;
			}
		}
		};
	}

	void Scanner::identifier()
	{
		while (isalnum(peek()) || peek() == '_')
			advance();

		static const std::unordered_map<std::string_view, TokenType> keywords =
		{
			{ "or",     TokenType::eOr      },
			{ "and",    TokenType::eAnd     },
			{ "struct", TokenType::eStruct  },
			{ "let",    TokenType::eLet,    },
			{ "if",     TokenType::eIf,     },
			{ "else",   TokenType::eElse,   },
			{ "return", TokenType::eReturn, },
			{ "while",  TokenType::eWhile,  },
			{ "for",    TokenType::eFor,    },
			{ "fn",     TokenType::eFun,    },
			{ "true",   TokenType::eTrue,   },
			{ "false",  TokenType::eFalse,  },
			{ "uniform",TokenType::eUniform,},
			{ "in",     TokenType::eInput,  },
			{ "out",    TokenType::eOutput, },
			{ "mut",    TokenType::eMut,    }
		};

		auto text = makeView();
		if (keywords.contains(text))
			tokens.emplace_back(keywords.at(text), currentLine, text);
		else
			tokens.emplace_back(TokenType::eIdentifier, currentLine, text);

	}

	void Scanner::number()
	{
		while (isdigit(peek()))
			advance();

		if (peek() == '.' && isdigit(peekNext()))
		{
			advance();
			while (isdigit(peek()))
				advance();

			tokens.emplace_back(TokenType::eReal, currentLine, makeView());
		}
		else
			tokens.emplace_back(TokenType::eInteger, currentLine, makeView());
	}

	bool Scanner::match(char expected)
	{
		if (isCursorAtEnd()) 
			return false;

		if (*cursor != expected)
			return false;

		cursor++;
		return true;
	}

	bool Scanner::isCursorAtEnd() const
	{
		return cursor >= source.end();
	}

	char Scanner::peekNext() const
	{
		if (cursor + 1 >= source.end())
			return '\0';

		return *(cursor + 1);
	}

	char Scanner::peek() const
	{
		if (isCursorAtEnd())
			return '\0';

		return *cursor;
	}

	std::string_view Scanner::makeView()
	{
		return std::string_view(start, cursor);
	}

	Scanner& Scanner::resetFlags()
	{
		flags = ScannerFlags::eNone;
		return *this;
	}

	Scanner& Scanner::addFlags(ScannerFlags flag)
	{
		flags = flags | flag;
		return *this;
	}

	std::string createContextMessage
	(
		const std::vector<Token>& tokens,
		size_t                    at,
		bool                      useColorCodes
	)
	{
		size_t      CONTEXT_TOKENS = 3;
		std::string message        = std::format("At line {}: ", tokens.at(at).line);
		auto        current_it     = tokens.begin() + at;
		auto        begin_it       = (at - CONTEXT_TOKENS < 0)
										? tokens.begin()
										: tokens.begin() + (at - CONTEXT_TOKENS);
		auto        end_it         = (at + CONTEXT_TOKENS >= tokens.size())
										? tokens.end()
										: tokens.begin() + (at + CONTEXT_TOKENS);
		size_t      hyphens        = message.length();
		
		for (auto it = begin_it; it != end_it; it++)
		{
			auto token = it->toStringView();
			if (useColorCodes && it == current_it)
			{
				message += std::format("\033[31m{}\033[0m ", token);
			}
			else
			{
				message += token;
				message += ' ';
			}

			if(it < current_it)
				hyphens += token.length() + 1;
		}

		message += '\n';
		message += std::string(hyphens, '-');
		if (useColorCodes)
			message += "\033[37m^ \033[35mhere \033[0m\n";
		else
			message += "^ here\n";
		
		return message;
	}
}

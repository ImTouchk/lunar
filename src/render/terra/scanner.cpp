#include <lunar/render/terra/token.hpp>

namespace Terra
{
	void scanShaderSource
	(
		Utils::Exp::Lexer& lexer,
		std::vector<Token>& tokens
	)
	{
		auto dictionary = Utils::Exp::Lexer::TokenDictionary
		{
			{ "{",        [&]() { tokens.emplace_back(TokenType::eLeftBrace, "{"); }},
			{ "}",        [&]() { tokens.emplace_back(TokenType::eRightBrace, "}"); } },
			{ "(",        [&]() { tokens.emplace_back(TokenType::eLeftParen, "("); } },
			{ ")",        [&]() { tokens.emplace_back(TokenType::eRightParen, ")"); } },
			{ ",",        [&]() { tokens.emplace_back(TokenType::eComma, ","); } },
			{ ".",        [&]() { tokens.emplace_back(TokenType::eDot, "."); } },
			{ "-",        [&]() { tokens.emplace_back(TokenType::eMinus, "-"); } },
			{ "+",        [&]() { tokens.emplace_back(TokenType::ePlus, "+"); } },
			{ ";",        [&]() { tokens.emplace_back(TokenType::eSemicolon, ";"); } },
			{ ":",        [&]() { tokens.emplace_back(TokenType::eDoubleColon, ":"); } },
			{ "*",        [&]() { tokens.emplace_back(TokenType::eStar, "*"); } },
			{ "!=",       [&]() { tokens.emplace_back(TokenType::eBangEqual, "!="); } },
			{ "!",        [&]() { tokens.emplace_back(TokenType::eBang, "!"); } },
			{ "==",       [&]() { tokens.emplace_back(TokenType::eEqualEqual, "=="); } },
			{ "=",        [&]() { tokens.emplace_back(TokenType::eEqual, "="); } },
			{ "<=",       [&]() { tokens.emplace_back(TokenType::eLessEqual, "<="); } },
			{ "<",        [&]() { tokens.emplace_back(TokenType::eLess, "<"); } },
			{ ">=",       [&]() { tokens.emplace_back(TokenType::eGreaterEqual, ">="); } },
			{ ">",        [&]() { tokens.emplace_back(TokenType::eGreater, ">"); } },
			{ "/",        [&]() { tokens.emplace_back(TokenType::eSlash, "/"); } },
			{ "or",       [&]() { tokens.emplace_back(TokenType::eOr, "||"); } },
			{ "and",      [&]() { tokens.emplace_back(TokenType::eAnd, "&&"); } },
			{ "struct",   [&]() { tokens.emplace_back(TokenType::eStruct, "struct"); } },
			{ "let",      [&]() { tokens.emplace_back(TokenType::eLet, "let"); } },
			{ "if",       [&]() { tokens.emplace_back(TokenType::eIf, "if"); } },
			{ "else",     [&]() { tokens.emplace_back(TokenType::eElse, "else"); } },
			{ "return",   [&]() { tokens.emplace_back(TokenType::eReturn, "return"); } },
			{ "while",    [&]() { tokens.emplace_back(TokenType::eWhile, "while"); } },
			{ "for",      [&]() { tokens.emplace_back(TokenType::eFor, "for"); } },
			{ "fn",       [&]() { tokens.emplace_back(TokenType::eFun, "fn"); } },
			{ "true",     [&]() { tokens.emplace_back(TokenType::eTrue, "true"); } },
			{ "false",    [&]() { tokens.emplace_back(TokenType::eFalse, "false"); } },
			{ "uniform",  [&]() { tokens.emplace_back(TokenType::eUniform, "uniform"); } },
			{ "in",       [&]() { tokens.emplace_back(TokenType::eInput, "input"); } },
			{ "out",      [&]() { tokens.emplace_back(TokenType::eOutput, "output"); } },
			{ "mut",      [&]() { tokens.emplace_back(TokenType::eMut, "mut"); } }
		};

		while (!lexer.isCursorAtEnd())
		{
			lexer.skipWhitespaces();

			if (lexer.parseFromDictionary(dictionary))
				continue;

			auto real = lexer.parseFloat();
			if (real.has_value())
			{
				tokens.emplace_back(TokenType::eReal, real.value());
				continue;
			}

			auto decimal = lexer.parseInt();
			if (decimal.has_value())
			{
				tokens.emplace_back(TokenType::eInteger, decimal.value());
				continue;
			}

			auto identifier = lexer.parseIdentifier();
			tokens.emplace_back(TokenType::eIdentifier, identifier);
		}

		tokens.emplace_back(TokenType::eEof);
	}
}

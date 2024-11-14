#pragma once
#include <lunar/api.hpp>
#include <lunar/exp/utils/lexer.hpp>
#include <lunar/debug.hpp>
#include <string_view>
#include <variant>
#include <format>
#include <vector>

namespace Utils::Exp
{
	enum class LUNAR_API TokenType
	{
		/* Single-character tokens */

		eLeftParen, eRightParen, eLeftBrace, eRightBrace,
		eComma, eDot, eMinus, ePlus, eSemicolon, eDoubleColon, eSlash, eStar,

		/* One or two character tokens */

		eBang, eBangEqual,
		eEqual, eEqualEqual,
		eGreater, eGreaterEqual,
		eLess, eLessEqual,

		/* Literals */

		eIdentifier, eReal, eInteger,

		/* Keywords */

		eAnd, eStruct, eElse, eFalse, eFun, eIf, eOr,
		eReturn, eTrue, eLet, eWhile, eFor, eUniform, eInput, eOutput,
		eMut,

		eWhitespace, eEof
	};

	struct LUNAR_API Token
	{
		using ValueType = std::string_view;

		size_t    line;
		TokenType type;
		ValueType value;

		Token(TokenType type, size_t line, const std::string_view& v) : line(line), type(type), value(v) {}
		Token(TokenType type, size_t line) : line(line), type(type), value("") {}

		bool operator==(TokenType type)
		{
			return this->type == type;
		}

		bool operator==(const std::string_view& value)
		{
			return this->value.compare(value) == 0;
		}

		std::string_view toStringView() const
		{
			return value;
		}

		std::string toString() const
		{
			return std::string(value);
		}

		operator std::string() { return toString(); }
	};

	LUNAR_API std::string createContextMessage
	(
		const std::vector<Token>& tokens,
		size_t                    at,
		bool                      useColorCodes = true
	);
}

#pragma once
#include <lunar/api.hpp>
#include <lunar/exp/utils/lexer.hpp>
#include <lunar/debug.hpp>
#include <string_view>
#include <variant>
#include <format>
#include <vector>

namespace Terra
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

		eEof
	};

	struct LUNAR_API Token
	{
		using ValueType = std::variant<
			float,
			int,
			std::string_view
		>;

		TokenType type;
		ValueType value;

		Token(TokenType type, const std::string_view& v) : type(type), value(v) {}
		Token(TokenType type, float f) : type(type), value(f) {}
		Token(TokenType type, int i) : type(type), value(i) {}
		Token(TokenType type) : type(type), value(-1) {}

		bool operator==(TokenType type)
		{
			return this->type == type;
		}

		bool operator==(const std::string_view& value)
		{
			if (this->value.index() != 2)
				return false;

			return std::get<std::string_view>(this->value)
				.compare(value) == 0;
		}

		std::string_view toStringView() const
		{
			DEBUG_ASSERT(value.index() == 2);
			return std::get<std::string_view>(value);
		}

		std::string toString() const
		{
			switch (value.index())
			{
			case 0: return std::format("{}", std::get<float>(value));
			case 1: return std::format("{}", std::get<int>(value));
			case 2: return std::format("{}", std::get<std::string_view>(value));
			}
		}

		operator std::string() { return toString(); }
	};

	LUNAR_API void scanShaderSource
	(
		Utils::Exp::Lexer& lexer,
		std::vector<Token>& tokens
	);
}

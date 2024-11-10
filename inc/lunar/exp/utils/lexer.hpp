#pragma once
#include <lunar/api.hpp>
#include <lunar/file/text_file.hpp>
#include <unordered_map>
#include <functional>

#include <string_view>
#include <string>

namespace Utils::Exp
{
	enum class LUNAR_API LexerFlagBits : uint32_t
	{
		eNone = 0,
		ePrintErrors = 1 << 0,
	};

	inline constexpr LUNAR_API LexerFlagBits operator|(LexerFlagBits a, LexerFlagBits b)
	{
		return static_cast<LexerFlagBits>(
			static_cast<uint32_t>(a) |
			static_cast<uint32_t>(b)
		);
	}

	inline constexpr LUNAR_API bool operator&(LexerFlagBits a, LexerFlagBits b)
	{
		return static_cast<uint32_t>(a) & static_cast<uint32_t>(b);
	}

	class LUNAR_API Lexer
	{
	public:
		using ParseHandler = std::function<void()>;
		struct TokenResolver
		{
			const char* name;
			ParseHandler resolver;
		};
		using TokenDictionary = std::vector<TokenResolver>;

		Lexer(std::string&& source, LexerFlagBits flags);
		~Lexer() = default;

		Lexer& skipWhitespaces();
		Lexer& setCursorPos(std::string::iterator pos);
		[[nodiscard]] std::string_view getSource() const;
		[[nodiscard]] std::string::iterator getCursor() const;
		[[nodiscard]] size_t getCursorLine() const;
		[[nodiscard]] bool isCursorAtEnd() const;

		[[nodiscard]] std::optional<float> parseFloat();
		[[nodiscard]] std::optional<int> parseInt();
		[[nodiscard]] std::optional<bool> parseBoolean();
		[[nodiscard]] std::string_view parseString(char delimiter = ' ');
		[[nodiscard]] std::string_view parseIdentifier();
		[[nodiscard]] bool parse(const char* string, ...);
		[[nodiscard]] bool parseLine(const char* string, ...);
		[[nodiscard]] bool parseFromDictionary(const TokenDictionary& dictionary);
		
	private:
		void parseError(const std::string& error);
		[[nodiscard]] bool vparse(const char* string, va_list args);

		LexerFlagBits flags;
		std::string source;
		std::string lastError;
		std::string::iterator pointer;
		size_t line;
		friend struct LexerBuilder;
	};

	struct LUNAR_API LexerBuilder
	{
		LexerBuilder() = default;
		~LexerBuilder() = default;

		LexerBuilder& appendTextFile(const Fs::Path& path);
		LexerBuilder& appendString(const std::string& string);
		LexerBuilder& enableErrorPrinting();
		[[nodiscard]] Lexer create();
	private:
		std::string source = "";
		LexerFlagBits flags = LexerFlagBits::eNone;
	};
}
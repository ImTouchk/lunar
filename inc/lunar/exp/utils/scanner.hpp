#pragma once
#include <lunar/api.hpp>
#include <lunar/exp/utils/token.hpp>
#include <initializer_list>
#include <unordered_map>
#include <string_view>
#include <functional>
#include <string>
#include <regex>

namespace Utils::Exp
{
	enum class LUNAR_API ScannerFlags : uint16_t
	{
		eNone            = 0,
		eScanWhitespaces = 1 << 1,
	};

	class LUNAR_API Scanner
	{
	public:
		Scanner(const std::string& source);
		Scanner(const std::string_view& source);

		Scanner& resetFlags();
		Scanner& addFlags(ScannerFlags flag);

		Scanner& run();
		[[nodiscard]] const std::vector<Token>& getResult() const;
		
	private:
		void next();
		void number();
		void identifier();
		char advance();
		[[nodiscard]] char peek() const;
		[[nodiscard]] char peekNext() const;
		[[nodiscard]] bool match(char expected);
		[[nodiscard]] bool isCursorAtEnd() const;
		[[nodiscard]] std::string_view makeView();

	private:
		ScannerFlags          flags       = ScannerFlags::eNone;
		size_t                currentLine = 1;
		std::string           source      = "";
		std::string::iterator start       = source.begin();
		std::string::iterator cursor      = source.begin();
		std::vector<Token>    tokens      = {};
	};

	inline ScannerFlags operator|(ScannerFlags a, ScannerFlags b)
	{
		return static_cast<ScannerFlags>(
			static_cast<uint16_t>(a) | static_cast<uint16_t>(b)
		);
	}

	inline bool operator&(ScannerFlags a, ScannerFlags b)
	{
		return static_cast<uint16_t>(a) & static_cast<uint16_t>(b);
	}
}

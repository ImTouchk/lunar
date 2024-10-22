#pragma once
#include <lunar/api.hpp>
#include <lunar/file/filesystem.hpp>
#include <lunar/utils/identifiable.hpp>
#include <lunar/exp/utils/lexer.hpp>
#include <vector>

namespace UI::Exp
{
	class LUNAR_API Element
	{
	public:
		Element() = default;
		~Element() = default;

		[[nodiscard]] std::string toPrettyString(int depth = 0) const;
		Element& setType(const std::string_view& type);
		Element& addAttribute(const std::string_view& name, const std::string_view& value);
		Element& addClass(const std::string_view& name);
		Element& appendWord(const std::string_view& text);

		char type[512];
		char id[64];
		char classes[10][64];
		size_t classCount = 0;
		char attributes[10][2][32];
		size_t attributeCount = 0;
		char innerText[4096];
		size_t textSize = 0;

		Element* parent;
		std::vector<Element> children;

	private:
		void copyStringView(const std::string_view& src, char* dst);
		friend class UiDom;
	};

	class LUNAR_API Dom
	{
	public:
		Dom();
		~Dom() = default;

		void parseSourceFile(const Fs::Path& path);
		void parseString(const std::string& source);

		std::string toPrettyString() const;
	private:
		bool tryParseElement(Utils::Exp::Lexer& lexer, Element& parent);

		std::unique_ptr<Element> root;
	};
}

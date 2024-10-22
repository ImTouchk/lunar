#include <lunar/exp/ui/dom.hpp>
#include <lunar/exp/utils/lexer.hpp>
#include <lunar/file/text_file.hpp>
#include <lunar/debug.hpp>
#include <algorithm>
#include <regex>

namespace UI::Exp
{
	void Element::copyStringView(const std::string_view& src, char* dst)
	{
		std::copy(src.begin(), src.end(), dst);
		dst[src.size()] = '\0';
	}

	Element& Element::appendWord(const std::string_view& text)
	{
		// TODO: bounds check

		auto val = std::string(text);
		val.erase(std::remove(val.begin(), val.end(), '\n'), val.end());

		auto view = std::string_view(val);

		copyStringView(view, innerText + textSize);
		innerText[textSize + view.size()] = ' ';
		innerText[textSize + view.size() + 1] = '\0';
		textSize += view.size() + 1;

		return *this;
	}

	Element& Element::setType(const std::string_view& type)
	{

		copyStringView(type, this->type);
		return *this;
	}

	Element& Element::addClass(const std::string_view& name)
	{
		copyStringView(name, classes[classCount]);
		classCount++;
		return *this;
	}

	Element& Element::addAttribute(const std::string_view& name, const std::string_view& value)
	{
		if (name.compare("id") == 0)
		{
			copyStringView(value, id);
			return *this;
		}

		if (name.compare("class") == 0)
		{
			auto split = [](const std::string_view& str) {
				std::vector<std::string_view> list = {};
				std::string_view token;

				size_t pos_end, pos_start = 0;
				while ((pos_end = str.find(' ', pos_start)) != std::string::npos)
				{
					token = str.substr(pos_start, pos_end - pos_start);
					pos_start = pos_end + 1;
					list.push_back(token);
				}
				
				list.push_back(str.substr(pos_start));
				return list;
			};

			auto classes = split(value);
			for (const auto& klass : classes)
				addClass(klass);
			
			return *this;
		}

		copyStringView(name, attributes[attributeCount][0]);
		copyStringView(value, attributes[attributeCount][1]);
		attributeCount++;
		return *this;
	}

	std::string Element::toPrettyString(int depth) const
	{
		auto begin_line = [&](std::string& str, int depth, const std::string_view& value) {
			auto val = std::string(value);
			auto spaces = std::string(depth * 2, ' ');

			str.append(spaces);
			std::regex_replace(val, std::regex("\n"), spaces);
			str.append(val);
		};

		auto append_line = [&](std::string& str, int depth, const std::string_view& value) {
			begin_line(str, depth, value);
			str.append("\n");
		};

		std::string res = "";
		if (strcmp(type, "_text") == 0)
		{
			if (textSize <= 0)
				return "";

			append_line(res, depth, innerText);
			return res;
		}

		if (attributeCount == 0)
			append_line(res, depth, std::format("<{}>", type));
		else
		{
			begin_line(res, depth, std::format("<{} ", type));
			for (size_t i = 0; i < attributeCount; i++)
				res.append(std::format("{}=\"{}\" ", attributes[i][0], attributes[i][1]));

			res.append(">\n");
		}

		for (const auto& child : children)
		{
			res.append(child.toPrettyString(depth + 1));
		}

		append_line(res, depth, std::format("</{}>", type));

		return res;
	}

	std::string Dom::toPrettyString() const
	{
		std::string res = "";
		for (const auto& child : root->children)
		{
			res += child.toPrettyString();
		}
		return res;
	}

	Dom::Dom()
		: root(std::make_unique<Element>())
	{
	}

	bool Dom::tryParseElement(Utils::Exp::Lexer& lexer, Element& parent)
	{
		auto copy_view = [&](const std::string_view& src, char* dst) {
			std::copy(src.begin(), src.end(), dst);
			dst[src.size()] = '\0';
		};
		
		std::string_view tag;
		if (!lexer.parseLine("<{:s_id}", &tag))
			return false;

		auto& child = parent.children.emplace_back();
		child.parent = &parent;
		child.setType(tag);
		
		std::string_view attrib_name, attrib_value;
		while (lexer.parseLine("{:s_id}=\"{:s_delim}\"", &attrib_name, '"', &attrib_value))
		{
			child.addAttribute(attrib_name, attrib_value);
		}

		if (!lexer.parseLine(">"))
			return false;

		auto end_tag = std::format("</{}>", tag);
		
		size_t text_elem_idx = child.children.size();
		child.children.emplace_back();
		child.children[text_elem_idx].setType("_text");

		while (true)
		{
			if (lexer.isCursorAtEnd())
				return false;

			if (lexer.parseLine(end_tag.c_str()))
				return true;

			auto& text_elem = child.children[text_elem_idx];

			auto start = lexer.getCursor();
			if (lexer.parseLine("<{:s_id}", &tag))
			{
				text_elem_idx = child.children.size();
				child.children.emplace_back();
				child.children[text_elem_idx].setType("_text");

				lexer.setCursorPos(start);
				tryParseElement(lexer, child);
				continue;
			}

			std::string_view str;
			if (lexer.parseLine("{:s}", &str))
			{
				text_elem.appendWord(str);
			}
		}

		return false;
	}

	void Dom::parseString(const std::string& source)
	{
		auto lexer = Utils::Exp::LexerBuilder()
			.appendString(source)
			.create();
		
		bool res;
		do { res = tryParseElement(lexer, *root.get()); } while (res);
	}

	void Dom::parseSourceFile(const Fs::Path& path)
	{
		auto file = Fs::TextFile(path);
		parseString(file.content);
	}
}

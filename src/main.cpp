#include <file/text_file.hpp>
#include <utils/argument_parser.hpp>
#include <utils/lexer.hpp>
#include <debug/log.hpp>
#include <ui/stylesheet.hpp>

int main(int argc, char* argv[])
{
    auto args = Utils::ArgumentParser(argc, argv);
    auto css = UI::StylesheetDictionary(Fs::dataDirectory().append("test.css"));
    return 1;
}

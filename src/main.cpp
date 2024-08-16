#include <file/text_file.hpp>
#include <utils/argument_parser.hpp>
#include <utils/lexer.hpp>
#include <debug/log.hpp>

int main(int argc, char* argv[])
{
    auto args = Utils::ArgumentParser(argc, argv);
    
    auto file = Fs::TextFile(Fs::dataDirectory().append("test.cfg"));
    auto lexer = Utils::Lexer(file.content);
    
    std::string key, value;
    if (lexer.consumeTemplate("{:s} = {:s}", &key, &value))
        DEBUG_LOG("{} = {}", key, value);

    return 1;
}

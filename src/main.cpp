#include <iostream>
#include <utils/argument_parser.hpp>
#include <debug/log.hpp>

int main(int argc, char* argv[])
{
    auto args = Utils::ArgumentParser(argc, argv);
    
    DEBUG_LOG("Hello, world!");
    DEBUG_WARN("Hello, world!");
    DEBUG_ERROR("Hello, world!");
    return 1;
}

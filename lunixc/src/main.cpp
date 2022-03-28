#include <fstream>
#include <iostream>
#include <filesystem>
#include <string_view>

using namespace std;

vector<string_view> parse_arguments(int argc, char* argv[])
{
    auto result = vector<string_view>();
    for(int i = 1; i < argc; i++)
    {
        auto argument = string_view(argv[i]);
        result.emplace_back(argument);
    }
    return result;
}

int main(int argc, char* argv[])
{
    auto arguments = parse_arguments(argc, argv);
    if(arguments.size() < 2)
    {
        printf("lunixc [input] [output] [opt: flags]\n");
        printf("|-------- flags: ");
        return 0;
    }

    auto in_path = filesystem::path(arguments.at(0));
    auto out_path = filesystem::path(arguments.at(1));

    if(!exists(in_path) || !is_regular_file(in_path))
    {
        printf("lunixc: Invalid arguments (input path must exist and be a file).\n");
        return 0;
    }

    printf("OK\n");
    return 0;
}
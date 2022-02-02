#include <iostream>
#include <filesystem>
#include <string_view>
#include <vector>

using namespace std;

void pack_dir(filesystem::path& in, filesystem::path& out);
void unpack_file(filesystem::path& in, filesystem::path& out);

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
    if(arguments.size() != 2)
    {
        printf("asset-packer [input] [output]\n");
        return 0;
    }

    auto in_path = filesystem::path(arguments.at(0));
    auto out_path = filesystem::path(arguments.at(1));

    if(!exists(in_path))
    {
        printf("Invalid arguments (input path does not exist).\n");
        return 0;
    }

    if(is_directory(in_path))
    {
        pack_dir(in_path, out_path);
    }
    else if(is_regular_file(in_path))
    {
        unpack_file(in_path, out_path);
    }

    return 0;
}
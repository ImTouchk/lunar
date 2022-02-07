#include <cstdint>
#include <fstream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

constexpr uint16_t MAGIC_NUMBER = 0b1010101010101010;

void extract_file_from_pkg(ifstream& input, filesystem::path& out)
{
    auto index = static_cast<uint64_t>(0);
    auto content_size = static_cast<uint64_t>(0);
    auto content_pos = static_cast<uint64_t>(0);
    auto name_len = static_cast<uint64_t>(0);
    auto name = string();

    input.read(reinterpret_cast<char*>(&index), sizeof(index));
    input.read(reinterpret_cast<char*>(&content_size), sizeof(content_size));
    input.read(reinterpret_cast<char*>(&content_pos), sizeof(content_pos));
    input.read(reinterpret_cast<char*>(&name_len), sizeof(name_len));

    name.resize(name_len);
    input.read(name.data(), streamsize(name_len));
    name.insert(0, out.generic_string());

    auto final_path = filesystem::path(name);
    filesystem::create_directory(final_path.parent_path());

    auto last_pos = input.tellg();
    auto file_data = vector<uint8_t>(content_size);

    input.seekg(streamsize(content_pos));
    input.read(reinterpret_cast<char*>(file_data.data()), content_size);
    input.seekg(last_pos);

    auto output = ofstream(name, ios::binary);
    if(!output.is_open())
    {
        printf("Could not open path \"%s\" for writing...\n", name.c_str());
        return;
    }

    output.write(reinterpret_cast<const char*>(file_data.data()), content_size);
    output.close();
}

void unpack_file(filesystem::path& in, filesystem::path& out)
{
    auto input = ifstream(in, ios::binary);
    if(!input.is_open())
    {
        printf("Could not open path \"%s\" for reading...\n", in.generic_string().c_str());
        return;
    }

    auto magic_number = static_cast<uint16_t>(0);
    input.read(reinterpret_cast<char*>(&magic_number), sizeof(magic_number));

    if(magic_number != MAGIC_NUMBER)
    {
        printf("Incompatible file type...\n");
        return;
    }

    auto package_name_size = static_cast<uint64_t>(0);
    input.read(reinterpret_cast<char*>(&package_name_size), sizeof(package_name_size));

    auto package_name = string();
    package_name.resize(package_name_size);
    input.read(package_name.data(), streamsize(package_name_size));
    printf("Extracting package %s...\n", package_name.c_str());

    filesystem::create_directory(out);

    auto total_files = static_cast<uint64_t>(0);
    input.read(reinterpret_cast<char*>(&total_files), sizeof(total_files));

    for(uint64_t i = 0; i < total_files; i++)
    {
        extract_file_from_pkg(input, out);
    }

    printf("Success! Unpacked file \"%s\".\n", in.generic_string().c_str());

    input.close();
}
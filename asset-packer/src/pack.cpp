#include <cstdint>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>

using namespace std;

constexpr uint16_t MAGIC_NUMBER = 0b1010101010101010;

vector<uint8_t> read_file_bytes(filesystem::path& path)
{
    auto input = ifstream(path, ios::ate | ios::binary);
    if(!input.is_open())
    {
        printf("Error: Could not open path \"%s\" for reading...\n", path.generic_string().c_str());
        return {};
    }

    auto file_size = input.tellg();
    auto bytes = vector<uint8_t>(file_size);

    input.seekg(ios::beg);
    input.read((char*)bytes.data(), file_size);
    input.close();

    return bytes;
}

vector<filesystem::path> get_files_in_directory(filesystem::path& path)
{
    auto result = vector<filesystem::path>();
    for(auto& entry : filesystem::recursive_directory_iterator(path))
    {
        if(entry.is_regular_file())
        {
            result.emplace_back(entry.path());
        }
    }
    return result;
}

void pack_dir(filesystem::path& in, filesystem::path& out, string_view& package_name)
{
    auto output = fstream(out, ios::binary | ios::out);
    if(!output.is_open())
    {
        printf("Could not open path \"%s\" for writing...\n", out.generic_string().c_str());
        return;
    }

    auto root_size = in.generic_string().size();
    auto file_metadatas = vector<uint64_t>();
    auto file_paths = get_files_in_directory(in);
    auto total_files = static_cast<uint64_t>(file_paths.size());

    output.write(reinterpret_cast<const char*>(&MAGIC_NUMBER), sizeof(MAGIC_NUMBER));

    auto package_name_size = static_cast<uint64_t>(package_name.size());
    output.write(reinterpret_cast<const char*>(&package_name_size), sizeof(package_name_size));
    output.write(reinterpret_cast<const char*>(package_name.data()), streamsize(package_name_size));

    output.write(reinterpret_cast<const char*>(&total_files), sizeof(total_files));

    {
        auto i = static_cast<uint64_t>(0);
        auto zero = static_cast<uint64_t>(0);
        for(auto& path : file_paths)
        {
            auto real_path = path.generic_string();

            auto name = real_path.substr(root_size + 1, real_path.size() - root_size + 1);
            auto name_size = static_cast<uint64_t>(name.size());

            file_metadatas.emplace_back(static_cast<uint64_t>(output.tellp()));

            output.write(reinterpret_cast<const char*>(&i), sizeof(i)); // file_index
            output.write(reinterpret_cast<const char*>(&zero), sizeof(zero)); // content_size
            output.write(reinterpret_cast<const char*>(&zero), sizeof(zero)); // content_pos
            output.write(reinterpret_cast<const char*>(&name_size), sizeof(name_size));
            output.write(name.data(), streamsize(name_size));

            i++;
        }
    }

    {
        auto i = static_cast<uint64_t>(0);
        for(auto& path : file_paths)
        {
            auto file_data = read_file_bytes(path);
            auto file_size = static_cast<uint64_t>(file_data.size());
            auto file_content_pos = static_cast<uint64_t>(output.tellp());

            output.seekp(file_metadatas.at(i) + sizeof(uint64_t));
            output.write(reinterpret_cast<const char*>(&file_size), sizeof(file_size));
            output.write(reinterpret_cast<const char*>(&file_content_pos), sizeof(file_content_pos));

            output.seekp(file_content_pos);
            output.write(reinterpret_cast<const char*>(file_data.data()), file_data.size());

            i++;
        }
    }

    printf("Success! Packaged directory \"%s\".\n", in.generic_string().c_str());

    output.close();
}
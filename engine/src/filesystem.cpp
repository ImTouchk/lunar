#include "utils/debug.hpp"
#include "io/filesystem.hpp"

#include <filesystem>
#include <string_view>
#include <unordered_map>
#include <fstream>
#include <vector>
#include <mutex>

// There is really no reason to have this filesystem in the first place;
// it probably is slower than having normal files, and you have to package things first,
// which does not even compress the files at all
// However, I have always wanted to make my own file format, and it seems that this time
// I have actually achieved it, more or less

// TODO: Have an option to disable file name caching (it is not necessary for those with fast storage)
// TODO: Ensure thread safety (I'm pretty sure it will glitch out if you try it right now)

struct PackageData
{
    struct FileData
    {
        uint64_t content_pos;
        uint64_t content_size;
    };

    std::filesystem::path realPath;
    std::vector<std::pair<std::string, FileData>> filesWithin;
};

std::unordered_map<std::string, PackageData> PACKAGES = {};
std::mutex MAP_MUTEX = {};

constexpr uint16_t MAGIC_NUMBER = 0b1010101010101010;

namespace CFilesystem
{
    void LoadPackage(const std::filesystem::path& path)
    {
        auto stream = std::ifstream(path, std::ios::binary);
        if(!stream.is_open())
        {
            CDebug::Error("Package Manager | Failed to load package \"{}\" (couldn't open file stream).", path.generic_string());
            return;
        }

        uint16_t magic_number = 0;
        stream.read(reinterpret_cast<char*>(&magic_number), sizeof(magic_number));

        if(magic_number != MAGIC_NUMBER)
        {
            CDebug::Log("Package Manager | Failed to load package \"{}\" (incompatible package version).", path.generic_string());
            return;
        }

        uint64_t package_name_size = 0;
        stream.read(reinterpret_cast<char*>(&package_name_size), sizeof(package_name_size));

        auto package_name = std::string();
        package_name.resize(package_name_size);
        stream.read(package_name.data(), std::streamsize(package_name_size));

        MAP_MUTEX.lock();
        auto& package_data = PACKAGES[package_name];
        MAP_MUTEX.unlock();

        package_data = {};
        package_data.realPath = path;
        package_data.filesWithin = {};

        uint64_t total_files = 0;
        stream.read(reinterpret_cast<char*>(&total_files), sizeof(total_files));

        for(uint64_t i = 0; i < total_files; i++)
        {
            uint64_t index;
            uint64_t content_size;
            uint64_t content_pos;
            uint64_t name_length;
            std::string file_name;

            stream.read(reinterpret_cast<char*>(&index), sizeof(index));
            stream.read(reinterpret_cast<char*>(&content_size), sizeof(content_size));
            stream.read(reinterpret_cast<char*>(&content_pos), sizeof(content_pos));
            stream.read(reinterpret_cast<char*>(&name_length), sizeof(name_length));

            file_name.resize(name_length);
            stream.read(file_name.data(), std::streamsize(name_length));

            CDebug::Log("filename: {}", file_name);

            package_data.filesWithin.push_back
            (
                std::pair<std::string, PackageData::FileData>
                (file_name, { content_pos, content_size })
            );
        }

        CDebug::Log(R"(Package Manager | Loaded package "{}" (real path: "{}") into memory.)", package_name, path.generic_string());

        stream.close();
    }

    void UnloadPackage(const std::string& name)
    {
        MAP_MUTEX.lock();
        if(PACKAGES.contains(name))
        {
            PACKAGES.erase(name);
            CDebug::Log("Package Manager | Unloaded package \"{}\".", name);
        }
        else
        {
            CDebug::Warn("Package Manager | UnloadPackage called on package that does not exist (\"{}\").", name);
        }
        MAP_MUTEX.unlock();
    }
}

CVirtualPath::CVirtualPath(const std::filesystem::path& path)
{
    auto path_string = path.generic_string();
    auto first_separator = path_string.find_first_of('/');
    if(first_separator == std::string::npos)
    {
        CDebug::Warn("Filestream | Path \"{}\" is invalid.", path_string);
        is_valid = false;
        return;
    }

    package_name = path_string.substr(0, first_separator);
    local_path = path_string.substr(first_separator + 1, path_string.size() - first_separator);
    full_path = path;
    is_valid = true;

    {
        std::lock_guard<std::mutex> lock(MAP_MUTEX);
        if(PACKAGES.contains(package_name))
        {
            if(local_path.generic_string().empty())
            {
                return;
            }

            bool path_found = false;

            auto& package = PACKAGES[package_name];
            for(auto& file : package.filesWithin)
            {
                if(file.first == local_path)
                {
                    path_found = true;
                    break;
                }
            }

            if(!path_found)
            {
                is_valid = false;
            }
        }
        else
        {
            is_valid = false;
        }
    }
}

bool CVirtualPath::exists() const
{
    return is_valid;
}

bool CVirtualPath::is_file() const
{
    return std::filesystem::is_regular_file(local_path);
}

bool CVirtualPath::is_directory() const
{
    return std::filesystem::is_directory(local_path);
}

std::vector<char> CVirtualPath::get_bytes()
{
    if (!is_valid)
    {
        CDebug::Error("Filesystem | CVirtualPath::get_bytes() called on invalid path \"{}\".", full_path.generic_string());
        throw std::runtime_error("Filesystem-VirtualPath-CalledOnInvalid");
        return {};
    }

    std::lock_guard<std::mutex> lock(MAP_MUTEX);
    if (!PACKAGES.contains(package_name))
    {
        // Package could have been unloaded in the meantime
        CDebug::Warn(R"(Filesystem | Path "{}" refers to package "{}" that no longer exists.)", full_path.generic_string(), package_name);
        is_valid = false;
        return {};
    }

    auto& package = PACKAGES[package_name];
    for (auto& file : package.filesWithin)
    {
        if (file.first == local_path)
        {
            uint64_t content_pos = file.second.content_pos;
            uint64_t content_size = file.second.content_size;

            auto stream = std::ifstream(package.realPath, std::ios::binary);
            if (!stream.is_open())
            {
                CDebug::Error(R"(Filesystem | Could not read path "{}" (could not open real path "{}").)", full_path.generic_string(), package.realPath.generic_string());
                return {};
            }

            auto content = std::vector<char>(content_size);
            stream.seekg(std::streamsize(content_pos));
            stream.read(content.data(), std::streamsize(content_size));
            stream.close();
            return content;
        }
    }

    // file was not found in the respective package
    CDebug::Warn(R"(Filesystem | Path "{}" no longer exists inside package "{}".)", full_path.generic_string(), package_name);
    is_valid = false;
    return {};
}


#pragma once
#include <filesystem>
#include <string_view>
#include <vector>

namespace CFilesystem
{
	void LoadPackage(const std::filesystem::path& path);
	void UnloadPackage(const std::string& name);
}

class CVirtualPath
{
public:
    CVirtualPath(const std::filesystem::path& path);
    ~CVirtualPath() = default;

    bool exists() const;
    bool is_file() const;
    bool is_directory() const;
    std::vector<char> get_bytes();

private:
    std::filesystem::path full_path;
    std::filesystem::path local_path; // local to the package
    std::string package_name;
    bool is_valid;
};

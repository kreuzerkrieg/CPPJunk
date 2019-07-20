#pragma once

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

void DownloadFile(const std::string& bucket, const fs::path& file_name, const fs::path& target_file);
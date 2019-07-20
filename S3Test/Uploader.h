#pragma once

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

void UploadFile(const std::string& bucket, const fs::path& file_name, const fs::path& source_file);
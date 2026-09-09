#pragma once
#include <vector>
#include <string>
#include "model.h"

struct FileInfo
{
    std::string name;
    int count;
};

void ensure_directories();

void save_csv(const DataSet& data, const std::string& path);

DataSet load_csv(const std::string& path);

std::vector<FileInfo> list_csv_files(const std::string& directory);

std::string with_csv_extension(const std::string& filename);

void write_csv(const std::string& path, const std::vector<std::string>& header, const std::vector<std::vector<std::string>>& rows);
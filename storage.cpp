#include "storage.h"
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

void save_csv(const DataSet& data, const std::string& path)
{
    std::ofstream file(path);

    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file for writing: " + path);
    }

    file << "temperature_C,shakes,elapsed_min,mass_loss_g\n";

    for (int i = 1; i <= data.size(); ++i)
    {
        const Record& record = data.get_record(i);
        file << record.temperature << ","
             << record.number_of_shakes << ","
             << record.elapsed_time << ","
             << record.mass_reduction << "\n";       
    }
}

static bool is_number(const std::string& s)
{
    try 
    {
        std::stod(s);
        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

DataSet load_csv(const std::string& path)
{
    std::ifstream file(path);
    
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file for reading: " + path);
    }

    DataSet data;
    
    std::string line;

    int line_number = 0;

    bool is_first_line = true;

    while (std::getline(file, line))
    {
        ++line_number;

        if (line.empty())
        {
            continue;
        }

        std::stringstream ss(line);
        std::string cell;
        std::vector<std::string> cells;
        std::vector<double> values;

        while (std::getline(ss, cell, ','))
        {
            cells.push_back(cell);
        }

        if (cells.size() != 4)
        {
            throw std::runtime_error("Invalid CSV format in file: " + path + " " + std::to_string(line_number) + "번째 줄");
        }

        if (is_first_line)
        {
            is_first_line = false;

            if (!is_number(cells[0]))
            {
                continue;
            }
        }

        for (const auto& c : cells)
        {
            try
            {
                values.push_back(std::stod(c));
            }
            catch (const std::invalid_argument&)
            {
                throw std::runtime_error("Invalid number format in CSV file: " + path + " " +  std::to_string(line_number) + "번째 줄");
            }
            catch (const std::out_of_range&)
            {
                throw std::runtime_error("Number out of range in CSV file: " + path + " " + std::to_string(line_number) + "번째 줄");
            }
        }

        data.add_record(Record{values[0], static_cast<int>(values[1]), values[2], values[3]});
    }

    return data;
}

void ensure_directories()
{
    fs::create_directories("data");
    fs::create_directories("output");
}

std::vector<FileInfo> list_csv_files(const std::string& directory)
{
    std::vector<FileInfo> result;

    if (!fs::exists(directory)) return {};

    for (const auto& entry : fs::directory_iterator(directory))
    {
        if (!entry.is_regular_file() || entry.path().extension() != ".csv")
            continue;

        FileInfo info;
        info.name = entry.path().filename().string();
        try
        {
            info.count = load_csv(entry.path().string()).size();
        }
        catch (const std::exception&)
        {
            info.count = -1;
        }
        result.push_back(info);

    }

    std::sort(result.begin(), result.end(), [](const FileInfo& a, const FileInfo& b) {return a.name < b.name; });

    return result;
}
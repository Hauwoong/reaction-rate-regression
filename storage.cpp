#include "storage.h"
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <cctype>

namespace fs = std::filesystem;

static void write_row(std::ofstream& file, const std::vector<std::string>& cells)
{
    for (size_t i = 0; i < cells.size(); ++i)
    {
        file << cells[i];

        if (i + 1 < cells.size())
            file << ",";
    }

    file << "\n";
}

void write_csv(const std::string& path, const std::vector<std::string>& header, const std::vector<std::vector<std::string>>& rows)
{
    for (const auto& row : rows)
        if (row.size() != header.size())
            throw std::invalid_argument("write_csv: 행의 칸 수가 헤더와 다릅니다");

    std::string full = with_csv_extension(path);

    std::ofstream file(full);

    if (!file.is_open())
        throw std::runtime_error("Failed to open file for writing: " + full);
    
    write_row(file, header);

    for (const auto& row: rows)
        write_row(file, row);
}

std::string with_csv_extension(const std::string& filename)
{
    std::string lower = filename;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) {return std::tolower(c);});

    if (fs::path(lower).extension() == ".csv")
        return filename;
    
    else 
        return filename + ".csv";
}

void save_csv(const DataSet& data, const std::string& path)
{
    std::string full = with_csv_extension(path);

    std::ofstream file(full);

    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file for writing: " + full);
    }

    file << "temperature_C,catalyst_g,h2o2_M,o2_rate_mL_s\n";

    for (int i = 1; i <= data.size(); ++i)
    {
        const Record& record = data.get_record(i);
        file << record.temperature << ","
             << record.catalyst_mass << ","
             << record.h2o2_conc << ","
             << record.o2_rate << "\n";
    }
}

// stod 는 성공 여부를 반환값으로 알려주지 않고 예외만 던진다.
// (반환값 0 으로는 판단할 수 없다 — 진짜 값이 0일 수도 있으므로.)
// 예외 없이 다음 줄에 도달했다는 것 자체가 성공의 증거다.
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

        // 첫 줄만 헤더인지 판별한다. 숫자로 안 바뀌면 헤더로 보고 건너뛴다.
        // 모든 줄에 이 검사를 적용하면 중간에 망가진 줄까지 조용히 삼켜서
        // "15건 넣었는데 12건만 읽혔다" 같은 일이 생긴다.
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

        data.add_record(Record{values[0], values[1], values[2], values[3]});
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


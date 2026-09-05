#pragma once
#include <iostream>
#include <vector>
#include <array>
#include <string>

using Vector = std::vector<double>;
using Matrix = std::vector<Vector>;

enum Column
{
    TEMP,
    SHAKES,
    ELAPSED,
    LOSS,
    COLUMN_COUNT
};

struct Record
{
    double temperature;
    int number_of_shakes;
    double elapsed_time;
    double mass_reduction;

    Vector factors() const;
    double get(Column c) const;
};

std::ostream& operator<<(std::ostream& os, const Record& record);

struct Range 
{
    double lo;
    double hi;
};

class DataSet
{
    std::vector<Record> records;
public:
    void add_record(const Record& record);
    int size() const;
    const Record& get_record(int index) const;
    std::array<Range, COLUMN_COUNT> ranges() const;
    Matrix design_matrix() const;
    Vector response() const;
    void delete_record(int index); // index-1 번째의 record를 삭제
    void clear();

};

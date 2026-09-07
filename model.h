#pragma once
#include <iostream>
#include <vector>
#include <array>
#include <string>
#include "linalg.h"

enum Column
{
    TEMP,
    CATALYST,
    H2O2,
    RATE,
    COLUMN_COUNT
};

struct Record
{
    double temperature;
    double catalyst_mass;
    double h2o2_conc;
    double o2_rate;

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

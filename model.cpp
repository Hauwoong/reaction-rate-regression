#include "model.h"
#include <stdexcept>

void DataSet::add_record(const Record& record)
{
    records.push_back(record);
}

int DataSet::size() const
{
    return static_cast<int>(records.size());
}

void DataSet::clear()
{
    records.clear();
}

const Record& DataSet::get_record(int index) const
{
    if (index < 1 || index > size())
    {
        throw std::out_of_range("Index out of range");
    }
    return records[index - 1];
}

void DataSet::delete_record(int index)
{
    if (index < 1 || index > size())
    {
        throw std::out_of_range("Index out of range");
    }
    records.erase(records.begin() + index - 1);
}

Vector Record::factors() const
{
    return { temperature, (double)number_of_shakes, elapsed_time };
}

double Record::get(Column c) const
{
    switch (c)
    {
        case TEMP:
            return temperature;
        case SHAKES:
            return static_cast<double>(number_of_shakes);
        case ELAPSED:
            return elapsed_time;
        case LOSS:
            return mass_reduction;
        default:
            throw std::invalid_argument("Invalid column");
    }
}

std::ostream& operator<<(std::ostream& os, const Record& record)
{
    os << "Record(temperature: " << record.temperature
       << ", number_of_shakes: " << record.number_of_shakes
       << ", elapsed_time: " << record.elapsed_time
       << ", mass_reduction: " << record.mass_reduction
       << ")";
    return os; 
}

Matrix DataSet::design_matrix() const
{
    Matrix matrix;
    for (const auto& record : records)
    {
        matrix.push_back(record.factors());
    }
    return matrix;
}

Vector DataSet::response() const
{
    Vector result;
    for (const auto& record : records)
    {
        result.push_back(record.mass_reduction);
    }
    return result;
}

std::array<Range, COLUMN_COUNT> DataSet::ranges() const
{
    std::array<Range, COLUMN_COUNT> ranges;

    if (records.empty())
    {
        for (int j = 0; j < COLUMN_COUNT; j++)
        {
            ranges[j].lo = 0.0;
            ranges[j].hi = 0.0;
        }
        return ranges;
    }

    for (int i = 0; i < static_cast<int>(records.size()); i++)
    {
        for (int j = 0; j < COLUMN_COUNT; j++)
        {
            double value = records[i].get(static_cast<Column>(j));
            
            if (i == 0)
            {
                ranges[j].lo = value;
                ranges[j].hi = value;
            }
            else
            {
                if (value < ranges[j].lo)
                {
                    ranges[j].lo = value;
                }

                if (value > ranges[j].hi)
                {
                    ranges[j].hi = value;
                }
            }
        }
    }
    return ranges;
}
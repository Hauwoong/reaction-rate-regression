#include <iostream>
#include <sstream>
#include <string>
#include <vector>

int main()
{
    std::string line = "25,10,5,1.85";

    std::stringstream ss(line);
    std::string cell;
    std::vector<std::string> cells;

    while (std::getline(ss, cell, ','))
    {
        cells.push_back(cell);
    }

    std::cout << "칸 개수: " << cells.size() << "\n";
    for (const auto& c : cells)
    {
        std::cout << c << "\n";
    }
}
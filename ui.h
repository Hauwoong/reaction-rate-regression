#pragma once
#include <string>
#include <vector>

struct InputCancelled {};

int display_width(const std::string& s);
std::string pad(const std::string& s, int width, bool right_align = false);
std::string trim(const std::string& s);

void print_menu();
double read_double(const std::string& label);
int read_int(const std::string& label);
bool ask_yes_no(const std::string& label);

std::string to_fixed(double value, int digits);
void print_table_line(const std::vector<int>& widths,
                      const std::string& left,
                      const std::string& mid,
                      const std::string& right);
                      
void print_table_row(const std::vector<int>& widths,
                     const std::vector<std::string>& cells,
                     bool rigth_align = false);
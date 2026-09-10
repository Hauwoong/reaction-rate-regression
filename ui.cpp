#include "ui.h"
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>

const int W[] = {5, 8, 10, 10, 12};

// UTF-8 은 첫 바이트만 보면 그 글자가 몇 바이트인지 알 수 있게 설계돼 있다.
//   0xxxxxxx → 1바이트 (영문, 숫자)
//   110xxxxx → 2바이트 (°, é 등)
//   1110xxxx → 3바이트 (한글, 박스 문자)
//   11110xxx → 4바이트 (이모지)
// 글자마다 건너뛰는 바이트 수가 다르므로 i 를 for 헤더가 아니라 본문에서 더한다.
// char 가 아니라 unsigned char 로 받는 것도 중요하다. char 는 대부분 부호가 있어
// 한글 바이트가 음수가 되면 아래 비교가 전부 틀어진다.
int display_width(const std::string& s)
{
    int width = 0;

    for (size_t i = 0; i < s.size();)
    {
        unsigned char c = s[i];

        if (c < 0x80)   {width += 1; i += 1; }
        else if (c < 0xE0)  {width += 1; i += 2; }
        else if (c < 0xF0)
        {
            // 3바이트에는 한글(2칸)과 박스 문자(1칸)가 섞여 있어 바이트 수만으로는
            // 구분할 수 없다. 세 바이트에서 코드포인트를 조립해 범위를 본다.
            if (i + 2 >= s.size())  { width += 1; i += 1; continue;}   // 잘린 문자 방어

            unsigned char b1 = s[i + 1];
            unsigned char b2 = s[i + 2];

            int cp = ((c & 0x0F) << 12) | ((b1 & 0x3F) << 6) | (b2 & 0x3F);

            if (cp >= 0xAC00 && cp <= 0xD7A3)     // 한글 완성형 (가 ~ 힣)
                width += 2;
            else                                   // 박스 문자(─│┌) 등은 1칸
                width += 1;

            i += 3;
        }
        else    {width +=2; i += 4; }
    }

    return width;
}

void print_menu()
{
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════╗\n";
    std::cout << "  ║   H2O2 Decomposition Rate Predictor v1.0 ║\n";
    std::cout << "  ║   과산화수소 분해 속도 예측 시스템       ║\n";
    std::cout << "  ╠══════════════════════════════════════════╣\n";
    std::cout << "  ║                                          ║\n";
    std::cout << "  ║   [1] 실험 데이터 입력                   ║\n";
    std::cout << "  ║   [2] 저장된 데이터 조회                 ║\n";
    std::cout << "  ║   [3] 회귀 모델 생성                     ║\n";
    std::cout << "  ║   [4] 산소 발생 속도 예측                ║\n";
    std::cout << "  ║   [5] 최적 반응 조건 탐색                ║\n";
    std::cout << "  ║   [6] 실험 가이드 출력                   ║\n";
    std::cout << "  ║   [7] CSV 내보내기                       ║\n";
    std::cout << "  ║   [0] 종료                               ║\n";
    std::cout << "  ║                                          ║\n";
    std::cout << "  ╚══════════════════════════════════════════╝\n";
}

std::string trim(const std::string& s)
{
    const std::string WS = " \t\r\n";

    size_t first = s.find_first_not_of(WS);
    if (first == std::string::npos)
        return "";
    
    size_t last = s.find_last_not_of(WS);
    return s.substr(first, last - first + 1);
}

double read_double(const std::string& label)
{
    while (true)
    {
        std::cout << "  " << label << ": ";

        std::string line;
        if (!std::getline(std::cin, line))
            throw InputCancelled {};
        
        try
        {
            return std::stod(trim(line));
        }
        catch(const std::exception& e)
        {
            std::cout << "숫자로 입력하세요.\n";
        }
        
    }
}

double read_double(const std::string& label, double default_value)
{
    while (true)
    {
        std::cout << "  " << label << " [" << to_fixed(default_value, 2) << "]: ";

        std::string line;
        if (!std::getline(std::cin, line))
            throw InputCancelled {};
        
        line = trim(line);

        if (line.empty())
            return default_value;

        try
        {
            return std::stod(line);
        }
        catch(const std::exception& e)
        {
            std::cout << "      숫자로 입력하세요.\n";
        }
           
    }
}

int read_int(const std::string& label)
{
    while (true)
    {
        std::cout << "  " << label << ": ";
        
        std::string line;
        if (!std::getline(std::cin, line))
            throw InputCancelled {};

        try
        {
            return std::stoi(trim(line));
        }
        catch(const std::exception& e)
        {
            std::cout << "숫자로 입력하세요.\n";
        }
        
    }
}

bool ask_yes_no(const std::string& label)
{
    while (true)
    {
        std::cout << "  " << label << " (y/n): ";

        std::string line;
        if (!std::getline(std::cin, line))
            throw InputCancelled {};
        
        line = trim(line);

        if (line == "y" || line == "Y") return true;
        if (line == "n" || line == "N") return false;

        std::cout << "y 또는 n으로 답하세요.\n";
    }
}

std::string pad(const std::string& s, int width, bool right_align)
{
    int space = width - display_width(s);

    if (space <= 0)
        return s;

    if (right_align)
        return std::string(space, ' ') + s;
    
    else
        return s + std::string(space, ' ');
}

std::string to_fixed(double value, int digits)
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(digits) << value;
    return oss.str();
}

void print_table_line(const std::vector<int>& widths, const std::string& left, const std::string& mid, const std::string& right)
{
    std::cout << "  " << left;

    for (size_t i = 0; i < widths.size(); ++i)
    {
        std::string bar = "";

        for (int j = 0; j < widths[i]+2; ++j)
            bar += "─";
        
        std::cout << bar;

        if (i + 1 < widths.size())
            std::cout << mid;
    }

    std::cout << right << "\n";
}

void print_table_row(const std::vector<int>& widths, const std::vector<std::string>& cells, bool right_align)
{
    if (cells.size() != widths.size())
        return;

    std::cout << "  │";

    for (size_t i = 0; i < cells.size(); ++i)
        std::cout  << " " << pad(cells[i], widths[i], right_align) << " │";

    std::cout << "\n";
}
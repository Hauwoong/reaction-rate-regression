#include <iostream>
#include <windows.h>
#include "model.h"
#include "storage.h"
#include <string>

struct InputCancelled {};

void print_menu()
{
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════╗\n";
    std::cout << "  ║   CO2 Solubility Predictor v1.0          ║\n";
    std::cout << "  ║   탄산음료 CO2 용해도 예측 시스템        ║\n";
    std::cout << "  ╠══════════════════════════════════════════╣\n";
    std::cout << "  ║                                          ║\n";
    std::cout << "  ║   [1] 실험 데이터 입력                   ║\n";
    std::cout << "  ║   [2] 저장된 데이터 조회                 ║\n";
    std::cout << "  ║   [3] 회귀 모델 생성                     ║\n";
    std::cout << "  ║   [4] CO2 잔존량 예측                    ║\n";
    std::cout << "  ║   [5] 최적 보관 조건 탐색                ║\n";
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
        std::cout << " " << label << ": ";

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

int read_int(const std::string& label)
{
    while (true)
    {
        std::cout << " " << label << ": ";
        
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

bool ask_yes_no(const std::string& label)
{
    while (true)
    {
        std::cout << " " << label << " 계속 입력? (y/n): ";

        std::string line;
        if (!std::getline(std::cin, line))
            throw InputCancelled {};
        
        try
        {
            line = trim(line);

            return line == "y";
        }
        catch(const std::exception& e)
        {
            std::cout << "y 또는 n으로 답하세요.\n";
        }
        
    }
}

void menu_input_data(DataSet& data)
{
    
}

void input_manually(DataSet& data)
{
    int count = 0;

    try
    {
        while (true)
        {
            std::cout << "\n === 실험 #" << (count + 1) << " ===\n";

            double temp    = read_double("온도 (°C)           ");
            int    shakes  = read_int   ("흔든 횟수 (회)      ");
            double elapsed = read_double("개봉 후 경과시간(분)");
            double loss    = read_double("질량 감소량 (g)     ");

            data.add_record({temp, shakes, elapsed, loss});
            ++count;

            std::cout << "계속 입력? (y/n): ";

            
        }
    }
    catch(const InputCancelled&)
    {
        std::cout << "\n 입력이 취소되었습니다.\n";
        return;
    }
    
    
}

void input_from_csv(DataSet& data)
{

}


int main()
{
    SetConsoleOutputCP(CP_UTF8);
    ensure_directories();

    DataSet data;

    while (true)
    {
        print_menu();
        std::cout << " >> 선택: ";

        std::string input;

        if (!std::getline(std::cin, input))
            break;

       
        std::string s = trim(input);

        if (s == "1")
        {

        }
        else if (s == "2")
        {

        }
        else if (s == "3")
        {

        }
        else if (s == "4")
        {

        }
        else if (s == "5")
        {

        }
        else if (s == "6")
        {

        }
        else if (s == "7")
        {

        }
        else if (s == "0")
        {
            break;
        }
        else 
        {
            std::cout << " 잘못된 선택입니다. 다시 입력해 주세요.\n";
        }
        
    }

    std::cout << "\n 프로그램을 종료 합니다.\n";
}
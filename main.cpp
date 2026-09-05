#include <iostream>
#include <windows.h>
#include "model.h"
#include "storage.h"
#include <string>

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
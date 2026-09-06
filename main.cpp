#include <iostream>
#include <windows.h>
#include "model.h"
#include "storage.h"
#include <string>
#include "ui.h"

void menu_view_data(DataSet& data)
{
    while (true)
    {
        if (data.size() == 0)
        {
            std::cout << "\n 데이터가 없습니다. [1] 메뉴에서 먼저 입력하세요.\n";
            return;
        }

        std::vector<int> W = {5, 8, 10, 10, 12};

        std::cout << "\n  ── 현재 로드된 데이터 (" << static_cast<int>(data.size()) << "건) ──\n";

        print_table_line(W, "┌", "┬", "┐");
        print_table_row (W, {"#", "온도°C", "흔든횟수", "경과시간", "질량감소(g)"});
        print_table_line(W, "├", "┼", "┤");

        for (int i = 1; i <= data.size(); ++i)
        {
            const Record& r = data.get_record(i);
            print_table_row(W, {std::to_string(i),
                to_fixed(r.temperature, 1),
                std::to_string(r.number_of_shakes),
                to_fixed(r.elapsed_time, 0),
                to_fixed(r.mass_reduction, 3)},
                true);
        }

        print_table_line(W, "└", "┴", "┘");

        const char* NAMES[] = {"온도", "흔든 횟수", "경과 시간", "질량 감소"};
        const char* UNITS[] = {"°C", "회", "분", "g"};
        const int DIGITS[] = {1, 0, 0, 2};

        auto r = data.ranges();

        std::cout << "\n  데이터 요약:\n";

        for (int c = 0; c < COLUMN_COUNT; ++c)
        {
            std::cout << "    " << pad(NAMES[c], 10)
              << " : " << to_fixed(r[c].lo, DIGITS[c])
              << " ~ " << to_fixed(r[c].hi, DIGITS[c])
              << " "   << UNITS[c] << "\n";
        }

        std::cout << "\n  [1] 삭제\n"
            << "  [2] 초기화\n"
            << "  [0] 원래 화면 되돌아오기\n";

        int input = read_int("번호를 입력하세요");

        if (input == 1)
        {
            int no = read_int("삭제할 행 번호");
            
            try
            {
                data.delete_record(no);
                std::cout << "  ✓ " << no << "번 행을 삭제했습니다.\n";
            }
            catch(const std::out_of_range&)
            {
                std::cout << "  ✗ " << no << "번 행이 없습니다.\n";
            }
            
            continue;
        }

        else if (input == 2)
        {
            if (ask_yes_no("정말 모든 데이터를 삭제할까요?"))
            {
                data.clear();
                std::cout << "  ✓ 데이터를 초기화했습니다.\n";
                return;
            }
            continue;
        }

        else
            return;
    }
}

void input_manually(DataSet& data)
{
    int count = 0;

    while (true)
    {
        std::cout << "\n  === 실험 #" << (count + 1) << " ===\n";

        double temp    = read_double("온도 (°C)           ");
        int    shakes  = read_int   ("흔든 횟수 (회)      ");
        double elapsed = read_double("개봉 후 경과시간(분)");
        double loss    = read_double("질량 감소량 (g)     ");

        data.add_record({temp, shakes, elapsed, loss});
        ++count;

        if (!ask_yes_no("계속 입력?"))
            break;
    }

    if (count == 0)
        return;

    std::cout << "\n  ✓ " << count << "건 입력 완료. 저장할 파일명: ";

    std::string filename;

    if (!std::getline(std::cin, filename))
        return;

    filename = trim(filename);

    if (filename.empty())
    {
        std::cout << "  ✗ 파일명을 입력하지 않아 저장하지 않았습니다.\n";
        return;
    }

    save_csv(data, "data/" + filename);
    std::cout << "  ✓ 저장 완료: ./data/" << with_csv_extension(filename) << "\n";
}

void input_from_csv(DataSet& data)
{
    auto files = list_csv_files("data");

    if (files.empty())
    {
        std::cout << "저장된 파일이 없습니다.\n";
        return;
    }
    
    int index = 1;

    for (const auto& file : files)
    {
        std::cout << "  [" << index << "] " << file.name << "  ";
        ++index;

        if (file.count == -1)
            std::cout << "(읽기 실패)\n";

        else
            std::cout << "(" << file.count << "건)\n";
    }

    while (true)
    {
        int choice = read_int("번호를 입력해주세요 ");

        if (choice >= 1 && choice <= static_cast<int>(files.size()))
        {
            try
            {
                DataSet loaded = load_csv("data/" + files[choice - 1].name);

                if (data.size() > 0)
                {
                    std::cout << "! 현재 " << data.size() << "건이 로드되어 있습니다.\n";

                    if (!ask_yes_no("기존 데이터에 덮어 쓰겠습니까? "))
                    return;
                }

                data = loaded;
                std::cout << "  ✓ " << files[choice - 1].name << " 로드 완료 (" << data.size() << "건)\n";
                return;
            }

            

            catch(const std::exception& e)
            {
                std::cout << "불러오기 실패: " << e.what() << "\n";
                return;
            }
        }

        else
        {
            std::cout << " 1 ~ " << files.size() << "사이로 입력하세요.\n";
        }
    }
}

void menu_input_data(DataSet& data)
{
    while (true)
    {
        std::cout << "\n  ── 실험 데이터 입력 ──────────────────\n\n";
        std::cout << "  입력 방식을 선택하세요:\n";
        std::cout << "    [1] 직접 입력\n";
        std::cout << "    [2] CSV 파일 불러오기\n";
        std::cout << "    [0] 메인 메뉴\n";
        std::cout << "  >> 선택: ";

        std::string input;

        if (!std::getline(std::cin, input))
            return;
        
        std::string s = trim(input);

        if (s == "1")
        {
            try
            {
                input_manually(data);
            }

            catch(const std::exception& e)
            {
                std::cout << "저장 실패: " << e.what() << "\n";
            }
            
            catch(const InputCancelled&)
            {
                std::cout << "잘못된 입력입니다!\n";
            }

            return;
        }

        else if (s == "2")
        {
            try
            {
                input_from_csv(data);
            }

            catch(const std::exception& e)
            {
                std::cout << "저장 실패: " << e.what() << "\n";
            }

            catch(const InputCancelled&)
            {
                std::cout << "잘못된 입력입니다!\n";
            }
            
            return;
        }

        else if (s == "0")
        {
            return;
        }

        else
        {
            std::cout << "잘못된 선택입니다.\n";
        }
    }
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
            menu_input_data(data);
        }
        else if (s == "2")
        {
            menu_view_data(data);
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
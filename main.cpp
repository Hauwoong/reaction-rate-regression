#include <iostream>
#include <cmath>
#include <windows.h>
#include "model.h"
#include "storage.h"
#include <string>
#include "ui.h"
#include "regression.h"

void menu_regression(DataSet& data, Model& model)
{
    std::cout << "\n  ── 다중 선형 회귀 분석 ───────────────\n\n";
    std::cout << "  데이터 건수: " << data.size() << "건\n";
    std::cout << "  변인 수    : 3개 (온도, 촉매 질량, H2O2 농도)\n";

    try
    {
        model = fit(data);
        std::cout << "\n  ✓ 회귀 모델 생성 완료!\n";
    }
    catch (const NotEnoughData&)
    {
        std::cout << "\n  ✗ 최소 " << COLUMN_COUNT << "건의 데이터가 필요합니다.\n";
        return;
    }
    catch (const SingularMatrix&)
    {
        std::cout << "\n  ✗ 변인 중 하나가 고정되어 있거나, 두 변인이 함께 움직였습니다.\n";
        return;
    }
    catch (const ConstantResponse&)
    {
        std::cout << "\n  ✗ 산소 발생 속도가 모두 같습니다. 측정값을 확인하세요.\n";
        return;
    }
    
    std::cout << "\n  ── 회귀 계수 ─────────────────────────\n\n";
    std::cout << "  속도(mL/s) = β0 + β1×온도 + β2×촉매 + β3×농도\n\n";

    const char* CNAMES[] = {"절편", "온도", "촉매", "농도"};
    const char* CUNITS[] = {"", "°C", "g", "M"};

    for (size_t i = 0; i < model.coefficients.size(); ++i)
    {
        double v = model.coefficients[i];
        std::string sign = (v >= 0) ? "+" : "";      // 음수는 '-'가 자동으로 붙는다
        std::string label = "β" + std::to_string(i) + " (" + CNAMES[i] + ")";

        std::cout << "    " << pad(label, 12) << " = " << sign << to_fixed(v, 4);

        if (i > 0)      // 절편에는 "1단위 증가" 설명이 없다
            std::cout << "   ← " << CNAMES[i] << " 1" << CUNITS[i]
                      << " 증가 시 " << sign << to_fixed(v, 3) << " mL/s";

        std::cout << "\n";
    }

    std::cout << "\n  ── 모델 정확도 ───────────────────────\n\n";
    std::cout << "    R² (결정계수) = " << to_fixed(model.r2, 4) << "\n";
    std::cout << "    RMSE          = " << to_fixed(model.rmse, 4) << "\n";

    std::cout << "\n  ── 변인 영향도 (실험 범위 기준) ──────\n\n";

    auto r = data.ranges();

    int    best        = 0;
    double best_impact = -1.0;

    for (int c = 0; c < COLUMN_COUNT - 1; ++c)   // 마지막 열(속도)은 출력이라 제외
    {
        double beta   = model.coefficients[c + 1];   // [0]은 절편이라 한 칸 밀림
        double span   = r[c].hi - r[c].lo;
        double impact = std::abs(beta) * span;

        if (impact > best_impact)
        {
            best_impact = impact;
            best = c;
        }

        std::cout << "    " << pad(CNAMES[c + 1], 6)
                  << " : "  << pad(to_fixed(std::abs(beta), 4), 7, true)
                  << " × "  << pad(to_fixed(span, 2), 6, true)
                  << " "    << pad(CUNITS[c + 1], 3)
                  << " = "  << pad(to_fixed(impact, 3), 7, true) << " mL/s\n";
    }

    std::cout << "\n  → 이 실험 범위에서는 " << CNAMES[best + 1] << "의 영향이 가장 큽니다.\n";
    std::cout << "     계수 크기만으로 비교하면 안 됩니다 — 변인마다 단위와 변화 폭이 다릅니다.\n";
}

void menu_view_data(DataSet& data)
{
    while (true)
    {
        if (data.size() == 0)
        {
            std::cout << "\n 데이터가 없습니다. [1] 메뉴에서 먼저 입력하세요.\n";
            return;
        }

        std::vector<int> W = {5, 8, 11, 11, 14};

        std::cout << "\n  ── 현재 로드된 데이터 (" << static_cast<int>(data.size()) << "건) ──\n";

        print_table_line(W, "┌", "┬", "┐");
        print_table_row (W, {"#", "온도°C", "촉매(g)", "H2O2(M)", "속도(mL/s)"});
        print_table_line(W, "├", "┼", "┤");

        for (int i = 1; i <= data.size(); ++i)
        {
            const Record& r = data.get_record(i);
            print_table_row(W, {std::to_string(i),
                to_fixed(r.temperature, 1),
                to_fixed(r.catalyst_mass, 2),
                to_fixed(r.h2o2_conc, 2),
                to_fixed(r.o2_rate, 3)},
                true);
        }

        print_table_line(W, "└", "┴", "┘");

        const char* NAMES[] = {"온도", "촉매 질량", "H2O2 농도", "산소 발생속도"};
        const char* UNITS[] = {"°C", "g", "M", "mL/s"};
        const int DIGITS[] = {1, 2, 2, 3};

        auto r = data.ranges();

        std::cout << "\n  데이터 요약:\n";

        for (int c = 0; c < COLUMN_COUNT; ++c)
        {
            std::cout << "    " << pad(NAMES[c], 14)
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

        double temp     = read_double("온도 (°C)              ");
        double catalyst = read_double("촉매 질량 (g)          ");
        double h2o2     = read_double("H2O2 초기 농도 (M)     ");
        double rate     = read_double("산소 발생 속도 (mL/s)  ");

        data.add_record({temp, catalyst, h2o2, rate});
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
    Model model;

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
            menu_regression(data, model);
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
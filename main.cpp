// main.cpp — 메뉴 화면과 프로그램 흐름
//
// 계산은 전부 다른 모듈(regression, optimizer, linalg)이 하고, 이 파일은
// "무엇을 물어보고 무엇을 보여줄지"만 담당한다.
//
// 프로그램 전체가 공유하는 상태는 main() 의 지역 변수 두 개뿐이다.
//   DataSet data   — 현재 로드된 실험 데이터
//   Model   model  — [3] 에서 만든 회귀 모델 (없으면 coefficients 가 비어 있음)
// 전역 변수를 쓰지 않고 참조로 넘기므로, 어떤 함수가 무엇을 바꾸는지
// 함수 시그니처만 봐도 알 수 있다.
//
// 입력 도중 EOF(Ctrl+Z)가 나면 ui 의 입력 함수들이 InputCancelled 를 던지고,
// main 의 각 메뉴 분기에서 잡아 프로그램이 죽지 않게 한다.

#include <iostream>
#include <cmath>
#include <windows.h>
#include <string>
#include <algorithm>
#include <filesystem>
#include "model.h"
#include "storage.h"
#include "ui.h"
#include "regression.h"
#include "optimizer.h"

static void export_data(const DataSet& data, const std::string& stem)
{
    if (data.size() == 0)
    {
        std::cout << "\n  ✗ 데이터가 없습니다.\n";
        return;
    }

    save_csv(data, "output/" + stem + "_data.csv");
    std::cout << "    ✓ " << stem << "_data.csv\n";
}

static void export_regression(const DataSet& data, const Model& model, const std::string& stem)
{   
    if (data.size() == 0)
    {
        std::cout << "\n  ✗ 데이터가 없습니다.\n";
        return;
    }

    if (model.coefficients.empty())
    {
        std::cout << "  ✗ 회귀 모델이 없습니다. [3] 을 먼저 실행하세요.\n";
        return;
    }

    const char* VARS[] = {"intercept", "temperature_C", "catalyst_g", "h2o2_M"};

    auto r = data.ranges();

    std::vector<std::vector<std::string>> rows;

    for (size_t i = 0; i < model.coefficients.size(); ++i)
    {
        // |β| × 실험 범위 — 단위가 다른 계수를 공정하게 비교하기 위한 값
        std::string influence = "";      // 절편은 영향도가 없다

        if (i > 0)
        {
            double span = r[i - 1].hi - r[i - 1].lo;   // coefficients[0]이 절편이라 한 칸 밀림
            influence = to_fixed(std::abs(model.coefficients[i]) * span, 10);
        }

        rows.push_back({"beta" + std::to_string(i), VARS[i],
                        to_fixed(model.coefficients[i], 10), influence});
    }

    // 지표 행도 헤더와 칸 수를 맞춰야 한다 (영향도는 빈 칸)
    rows.push_back({"metric", "r_squared", to_fixed(model.r2, 10), ""});
    rows.push_back({"metric", "rmse", to_fixed(model.rmse, 10), ""});
    rows.push_back({"metric", "n_samples", std::to_string(model.fitted.size()), ""});

    write_csv("output/" + stem + "_regression.csv",
              {"term", "variable", "value", "influence"}, rows);

    std::cout << "    ✓ " << stem << "_regression.csv\n";
}

static void export_prediction(const DataSet& data, const Model& model, const std::string& stem)
{
    if (model.fitted.size() != static_cast<size_t>(data.size()))
    {
        std::cout << "  ✗ 데이터가 변경되었습니다. [3] 에서 회귀 모델을 다시 생성하세요.\n";
        return;
    }

    std::vector<std::vector<std::string>> rows;

    for (int i = 1; i <= data.size(); ++i)
    {
        const Record& rec = data.get_record(i);

        rows.push_back({
            to_fixed(rec.temperature, 10),
            to_fixed(rec.catalyst_mass, 10),
            to_fixed(rec.h2o2_conc, 10),
            to_fixed(rec.o2_rate, 10),
            to_fixed(model.fitted[i -1], 10),
            to_fixed(model.residuals[i - 1], 10)
        });
    }

    write_csv("output/" + stem + "_prediction.csv", {"temperature_C", "catalyst_g", "h2o2_M", "o2_rate_mL_s", "predicted", "residual"}, rows);

    std::cout << "    ✓ " << stem << "_prediction.csv\n";
}

static void export_surface(const DataSet& data, const Model& model, const std::string& stem)
{
    if (data.size() == 0)
    {
        std::cout << "  ✗ 데이터가 없습니다.\n";
        return;
    }

    if (model.coefficients.empty())
    {
        std::cout << "  ✗ 회귀 모델이 없습니다. [3] 을 먼저 실행하세요.\n";
        return;
    }

    auto r = data.ranges();
    std::vector<Range> bounds(r.begin(), r.begin() + 3);   // array 4개 → vector 3개

    auto grid = grid_search(model, bounds, 20);            // 21³ = 9,261개

    std::vector<std::vector<std::string>> rows;

    for (const auto& c : grid)
        rows.push_back({
            to_fixed(c.factors[0], 10),
            to_fixed(c.factors[1], 10),
            to_fixed(c.factors[2], 10),
            to_fixed(c.rate, 10)
        });

    write_csv("output/" + stem + "_surface.csv",
              {"temperature_C", "catalyst_g", "h2o2_M", "predicted"},
              rows);

    std::cout << "    ✓ " << stem << "_surface.csv  (" << rows.size() << "행)\n";
}

void menu_export(const DataSet& data, const Model& model)
{
    std::cout << "\n  ── CSV 내보내기 ──────────────────────\n";

    std::string choice;

    while (true)
    {
        std::cout << "\n  내보낼 항목을 선택하세요:\n";
        std::cout << "    [1] 원본 실험 데이터\n";
        std::cout << "    [2] 회귀 결과 (계수 + 정확도 + 영향도)\n";
        std::cout << "    [3] 예측값 포함 데이터 (실측 vs 예측)\n";
        std::cout << "    [4] 3D 표면용 격자 데이터\n";
        std::cout << "    [5] 전체 내보내기\n";
        std::cout << "    [0] 메인 메뉴\n";
        std::cout << "  >> 선택: ";

        std::string line;

        if (!std::getline(std::cin, line))
            return;

        choice = trim(line);

        if (choice == "0")
            return;

        if (choice == "1" || choice == "2" || choice == "3" || choice == "4" || choice == "5")
            break;

        std::cout << "      0 ~ 5 중에서 선택하세요.\n";
    }

    std::cout << "\n  파일명 (확장자 없이): ";

    std::string stem;

    if (!std::getline(std::cin, stem))
        return;

    stem = trim(stem);

    if (stem.empty())
    {
        std::cout << "  ✗ 파일명을 입력하지 않아 저장하지 않았습니다.\n";
        return;
    }

    bool all = (choice == "5");

    // 절대 경로를 한 번만 찍는다. MATLAB 에서 이 폴더를 찾아가야 하므로
    // "./output/" 처럼 모호한 표기로는 부족하다.
    std::cout << "\n  저장 위치: " << std::filesystem::absolute("output").string() << "\n\n";

    try
    {
        if (all || choice == "1") export_data(data, stem);
        if (all || choice == "2") export_regression(data, model, stem);
        if (all || choice == "3") export_prediction(data, model, stem);
        if (all || choice == "4") export_surface(data, model, stem);
    }
    catch (const std::exception& e)
    {
        std::cout << "  ✗ 저장 실패: " << e.what() << "\n";
        return;
    }

    std::cout << "\n  → MATLAB 에서 visualize.m 을 열어 맨 위 stem 을 다음으로 바꾸고 실행하세요:\n";
    std::cout << "       stem = '" << stem << "';\n";
}

void menu_experiment_guide(const DataSet& data)
{
    std::cout << "\n  ── 실험 설계 가이드 ──────────────────\n\n";
    std::cout << "  회귀 모델의 신뢰도를 높이기 위한\n";
    std::cout << "  최소 실험 조건 조합을 추천합니다.\n";

    const char* GNAMES[] = {"온도 (°C)     ", "촉매 (g)      ", "H2O2 (M)      "};
    const char* GUNITS[] = {"°C", "g", "M"};
    const int GDIGITS[] = {1,2,2};

    const double DEF_LO[] = {20.0, 0.10, 0.5};
    const double DEF_HI[] = {50.0, 0.40, 2.0};

    auto r = data.ranges();
    bool has_data = (data.size() > 0);

    Matrix levels;

    for (int c = 0; c < COLUMN_COUNT - 1; ++c)
    {
    double def_lo = has_data ? r[c].lo : DEF_LO[c];
    double def_hi = has_data ? r[c].hi : DEF_HI[c];

    double lo = read_double(std::string(GNAMES[c]) + "최소", def_lo);
    double hi = read_double(std::string(GNAMES[c]) + "최대", def_hi);

    if (lo > hi)
        std::swap(lo, hi);        // menu_optimize 처럼 물어봐도 됩니다

    double step = (hi - lo) / 3.0;    // 4개 점 → 3등분

    Vector v;
    for (int k = 0; k < 4; ++k)
        v.push_back(lo + k * step);

    levels.push_back(v);
    }

    std::cout << "\n  변인 수준 설정:\n";

    for (int c = 0; c < COLUMN_COUNT - 1; ++c)
    {
        std::cout << "    " << pad(GNAMES[c], 14) << ": ";

        for (int k = 0; k < 4; ++k)
        {
            std::cout << to_fixed(levels[c][k], GDIGITS[c]);
            if (k < 3) std::cout << ", ";
        }

        std::cout << " " << GUNITS[c] << "  (4수준)\n";
    }

    std::cout << "\n  ── 추천 실험 계획 (부분 요인 설계) ───\n\n";
    std::cout << "  총 추천 실험 수: 16회\n";
    std::cout << "  (완전 요인 설계 64회 중 핵심 조합)\n\n";

    // 라틴방격으로 16개 조합 생성 — 이 규칙은 여기 한 곳에만 있다.
    //
    // 4수준 3변인을 전부 실험하면 4³ = 64회다. 온도(i) × 촉매(j) 를 완전 조합
    // (16칸)으로 두고 농도를 (i+j) mod 4 로 배정하면, 어떤 두 변인을 짝지어 봐도
    // 모든 조합이 정확히 한 번씩 나타나는 16회 설계가 된다.
    //
    // 이렇게 해야 변인들이 서로 겹쳐 움직이지 않는다. 예를 들어 온도를 올릴 때마다
    // 촉매도 같이 올리면 속도 변화가 무엇 때문인지 구분할 수 없고, 수학적으로도
    // XᵀX 가 특이행렬이 되어 회귀 계산 자체가 불가능해진다.
    DataSet plan;

    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
        {
            int k = (i + j) % 4;
            plan.add_record({levels[0][i], levels[1][j], levels[2][k], 0.0});
            //                                                          ↑ 속도는 실험 후에 채운다
        }

    std::vector<int> W = {6, 8, 10, 10};

    print_table_line(W, "┌", "┬", "┐");
    print_table_row (W, {"번호", "온도°C", "촉매(g)", "H2O2(M)"});
    print_table_line(W, "├", "┼", "┤");

    for (int n = 1; n <= plan.size(); ++n)
    {
        const Record& p = plan.get_record(n);
        print_table_row(W, {
            std::to_string(n),
            to_fixed(p.temperature, GDIGITS[0]),
            to_fixed(p.catalyst_mass, GDIGITS[1]),
            to_fixed(p.h2o2_conc, GDIGITS[2])
        }, true);
    }

    print_table_line(W, "└", "┴", "┘");

    std::cout << "\n  이 표를 출력해서 실험할 때 체크리스트로 사용하세요.\n";

    if (!ask_yes_no("\n  이 계획을 CSV로 저장할까요?"))
        return;

    std::cout << "  저장할 파일명: ";

    std::string filename;

    if (!std::getline(std::cin, filename))
        return;

    filename = trim(filename);

    if (filename.empty())
    {
        std::cout << "  ✗ 파일명을 입력하지 않아 저장하지 않았습니다.\n";
        return;
    }

    try
    {
        save_csv(plan, "data/" + filename);
        std::cout << "  ✓ 저장 완료: ./data/" << with_csv_extension(filename) << "\n";
        std::cout << "     실험 후 o2_rate_mL_s 열을 채워서 [1] → [2] 로 불러오세요.\n";
    }
    catch (const std::exception& e)
    {
        std::cout << "  ✗ 저장 실패: " << e.what() << "\n";
    }

}

void menu_optimize(const DataSet& data, const Model& model)
{
    std::cout << "\n  ── 최적 반응 조건 탐색 ───────────────\n";

    if (data.size() == 0)
    {
        std::cout << "\n  ✗ 데이터가 없습니다. [1] 메뉴에서 먼저 입력하세요.\n";
        return;
    }

    if (model.coefficients.empty())
    {
        std::cout << "\n  ✗ 먼저 [3] 에서 회귀 모델을 생성하세요.\n";
        return;
    }

    auto r = data.ranges();

    const char* FNAMES[] = {"온도 (°C)     ", "촉매 (g)      ", "H2O2 (M)      "};

    std::vector<Range> bounds;

    for (int c = 0; c < COLUMN_COUNT - 1; ++c)
    {
        double lo = read_double(std::string(FNAMES[c]) + "최소", r[c].lo);
        double hi = read_double(std::string(FNAMES[c]) + "최대", r[c].hi);

        if (lo > hi)
        {
            std::cout << "\n  ✗ 최솟값이 최댓값보다 더 큽니다!\n";
            std::cout << "  최댓값과 최솟값을 서로 바꾸겠습니까? 아니면 다시 원래 화면으로 돌아가겠습니까?.\n";

            if (ask_yes_no("  바꾸려면 / 되돌아가려면"))
                std::swap(lo,hi);
            
            else
                return;
        }

        if (lo < r[c].lo || hi > r[c].hi)
            std::cout << "  ! 실험 범위(" << r[c].lo << "~" << r[c].hi << ")를 벗어난 구간이 포함됩니다. 예측이 부정확할 수 있습니다.\n";

        bounds.push_back({lo, hi});
    }

    int divisions = read_int("탐색 분할 수 (권장 20)");

    while (divisions < 1 || divisions > 100)
    {
        std::cout << "      1 ~ 100 사이로 입력하세요.\n";
        divisions = read_int("탐색 분할 수 (권장 20)"); 
    }

    int points = divisions + 1;
    std::cout << "\n  → " << points << " × " << points << " × " << points
          << " = " << (points * points * points) << "개 조합을 탐색합니다.\n";

    // 4. 탐색 모드 선택
    int    mode   = 0;
    double target = 0.0;      // 모드 2, 3 에서만 쓰인다

    while (true)
    {
        std::cout << "\n  탐색 모드를 선택하세요:\n";
        std::cout << "    [1] 반응 속도가 가장 빠른 조건\n";
        std::cout << "    [2] 목표 속도에 가장 가까운 조건\n";
        std::cout << "    [3] 목표 속도 이상이면서 촉매를 가장 적게 쓰는 조건\n";
        std::cout << "    [0] 메인 메뉴\n";
        std::cout << "  >> 선택: ";

        std::string line;

        if (!std::getline(std::cin, line))
            return;

        std::string s = trim(line);

        if (s == "0")
            return;

        if (s == "1" || s == "2" || s == "3")
        {
            mode = std::stoi(s);

            if (mode != 1)      // 모드 1은 목표값이 필요 없다
                target = read_double("목표 산소 발생 속도 (mL/s)");

            break;
        }

        std::cout << "      0 ~ 3 중에서 선택하세요.\n";
    }

    // 5. 탐색
    std::vector<Candidate> all = grid_search(model, bounds, divisions);

    // 6. 모드별 정렬 / 필터
    if (mode == 1)
    {
        std::sort(all.begin(), all.end(),
                  [](const Candidate& a, const Candidate& b) { return a.rate > b.rate; });
    }
    else if (mode == 2)
    {
        std::sort(all.begin(), all.end(),
                  [target](const Candidate& a, const Candidate& b)
                  { return std::abs(a.rate - target) < std::abs(b.rate - target); });
    }
    else
    {
        std::vector<Candidate> ok;

        for (const auto& c : all)
            if (c.rate >= target)
                ok.push_back(c);

        if (ok.empty())
        {
            std::cout << "\n  ✗ 이 탐색 범위에서는 " << to_fixed(target, 3)
                      << " mL/s 이상을 낼 수 없습니다.\n";
            std::cout << "     범위를 넓히거나 목표를 낮춰보세요.\n";
            return;
        }

        std::sort(ok.begin(), ok.end(),
                  [](const Candidate& a, const Candidate& b)
                  {
                      if (a.factors[1] != b.factors[1])
                          return a.factors[1] < b.factors[1];   // 1순위: 촉매 적은 순

                      return a.rate > b.rate;                    // 2순위: 속도 빠른 순
                  });

        all = ok;
    }

    // 7. 결과 표
    std::cout << "\n  ── 결과: Top 5 ───────────────────────\n\n";

    std::vector<int> W = {6, 8, 11, 11, 14};

    print_table_line(W, "┌", "┬", "┐");
    print_table_row (W, {"순위", "온도°C", "촉매(g)", "H2O2(M)", "속도(mL/s)"});
    print_table_line(W, "├", "┼", "┤");

    for (size_t i = 0; i < 5 && i < all.size(); ++i)
    {
        const Candidate& c = all[i];
        print_table_row(W, {
            std::to_string(i + 1),
            to_fixed(c.factors[0], 1),
            to_fixed(c.factors[1], 3),
            to_fixed(c.factors[2], 3),
            to_fixed(c.rate, 3)
        }, true);
    }

    print_table_line(W, "└", "┴", "┘");

    // 8. 모드별 결론
    const Candidate& top = all.front();

    if (mode == 1)
    {
        std::cout << "\n  → 가장 빠른 조건은 " << to_fixed(top.factors[0], 1) << " °C, "
                  << to_fixed(top.factors[1], 3) << " g, "
                  << to_fixed(top.factors[2], 3) << " M 에서 "
                  << to_fixed(top.rate, 3) << " mL/s 입니다.\n";
        std::cout << "     세 계수가 모두 양수라 탐색 범위의 최댓값 쪽이 답이 됩니다.\n";
    }
    else if (mode == 2)
    {
        std::cout << "\n  → 목표 " << to_fixed(target, 3) << " mL/s 에 가장 가까운 조건은 "
                  << to_fixed(top.factors[0], 1) << " °C, "
                  << to_fixed(top.factors[1], 3) << " g, "
                  << to_fixed(top.factors[2], 3) << " M (오차 "
                  << to_fixed(std::abs(top.rate - target), 3) << " mL/s) 입니다.\n";
        std::cout << "     서로 다른 조합이 비슷한 속도를 내므로, 실험 여건에 맞는 것을 고르세요.\n";
    }
    else
    {
        std::cout << "\n  → 목표 " << to_fixed(target, 3) << " mL/s 를 달성하는 최소 촉매량은 "
                  << to_fixed(top.factors[1], 3) << " g 입니다.\n";
        std::cout << "     이때 조건은 " << to_fixed(top.factors[0], 1) << " °C, "
                  << to_fixed(top.factors[2], 3) << " M 이고 예측 속도는 "
                  << to_fixed(top.rate, 3) << " mL/s 입니다.\n";
    }
}

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

void menu_predict(const DataSet& data, const Model& model)
{
    std::cout << "\n  ── 산소 발생 속도 예측 ───────────────\n";

    if (model.coefficients.empty())
    {
        std::cout << "\n  ✗ 먼저 [3] 에서 회귀 모델을 생성하세요.\n";
        return;
    }

    const char* FNAMES[] = {"온도", "촉매 질량", "H2O2 농도"};
    const char* FUNITS[] = {"°C", "g", "M"};
    const int   FDIGITS[] = {1, 2, 2};

    auto range = data.ranges();

    try
    {
        while (true)
        {
            std::cout << "\n  조건을 입력하세요:\n";

            Vector factors = {
                read_double("온도 (°C)              "),
                read_double("촉매 질량 (g)          "),
                read_double("H2O2 초기 농도 (M)     ")
            };

            // 실험 범위를 벗어나면 외삽이라 신뢰할 수 없다
            for (int c = 0; c < COLUMN_COUNT - 1; ++c)
            {
                if (factors[c] < range[c].lo || factors[c] > range[c].hi)
                {
                    std::cout << "\n  ! " << FNAMES[c] << " " << to_fixed(factors[c], FDIGITS[c])
                              << " " << FUNITS[c] << " 는 실험 범위("
                              << to_fixed(range[c].lo, FDIGITS[c]) << " ~ "
                              << to_fixed(range[c].hi, FDIGITS[c]) << ")를 벗어납니다.";
                }
            }

            double rate = model.predict(factors);

            std::cout << "\n\n  ── 예측 결과 ─────────────────────────\n\n";
            std::cout << "    예측 산소 발생 속도: " << to_fixed(rate, 3) << " mL/s\n";

            if (rate < 0.0)
                std::cout << "\n  ! 예측값이 음수입니다. 실제로는 반응이 거의 일어나지 않는 조건이거나,\n"
                          << "    모델이 이 조건을 설명하지 못하는 것입니다.\n";

            if (!ask_yes_no("\n  다른 조건으로 다시 예측?"))
                return;
        }
    }
    catch (const InputCancelled&)
    {
        std::cout << "\n  입력이 취소되었습니다.\n";
    }
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
    // 절대 경로로 보여준다. "data 폴더"라고만 하면 어디를 말하는지 알 수 없다.
    std::string folder = std::filesystem::absolute("data").string();

    auto files = list_csv_files("data");

    if (files.empty())
    {
        std::cout << "\n  ✗ 불러올 CSV 파일이 없습니다.\n\n";
        std::cout << "     아래 폴더에 CSV 파일을 넣은 뒤 다시 시도하세요:\n";
        std::cout << "       " << folder << "\n\n";
        std::cout << "     파일 형식 — 열 순서가 중요하며, 헤더 줄은 있어도 없어도 됩니다:\n";
        std::cout << "       온도(°C), 촉매 질량(g), H2O2 초기 농도(M), 산소 발생 속도(mL/s)\n\n";
        std::cout << "       20,0.10,0.5,1.68\n";
        std::cout << "       30,0.10,0.5,1.87\n\n";
        std::cout << "     엑셀에서 만들 때는 [CSV UTF-8 (쉼표로 분리)] 형식으로 저장하세요.\n";
        std::cout << "     [6] 실험 가이드에서 빈 계획표를 만들어 채우는 방법도 있습니다.\n";
        return;
    }

    std::cout << "\n  " << folder << "\n\n";

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
            menu_predict(data, model);
        }
        else if (s == "5")
        {
            try
            {
                menu_optimize(data, model);
            }
            catch (const InputCancelled&)
            {
                std::cout << "\n  입력이 취소되었습니다.\n";
            }
        }
        else if (s == "6")
        {
            try
            {
                menu_experiment_guide(data);
            }
            catch (const InputCancelled&)
            {
                std::cout << "\n  입력이 취소되었습니다.\n";
            }
        }
        else if (s == "7")
        {
            try
            {
                menu_export(data, model);
            }
            catch (const InputCancelled&)
            {
                std::cout << "\n  입력이 취소되었습니다.\n";
            }
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
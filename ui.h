// ui.h — 화면 출력과 사용자 입력 도구
//
// 이 파일은 model 을 모른다(Record 나 DataSet 을 인자로 받지 않는다).
// 문자열과 숫자만 다루므로 다른 프로그램에서도 그대로 쓸 수 있다.

#pragma once
#include <string>
#include <vector>

// 입력이 끊겼을 때(EOF, 예: Ctrl+Z) 던진다.
// 여러 겹의 루프 안쪽에서 한 번에 메뉴까지 빠져나오기 위한 신호용 타입이라
// 멤버가 없다. 메뉴 함수에서 catch 한다.
struct InputCancelled {};

// ── 문자열 도구 ────────────────────────────────

// 터미널에서 차지하는 칸 수. std::string::size() 는 바이트 수라 쓸 수 없다.
// UTF-8 에서 한글 한 글자는 3바이트인데 화면에서는 2칸을 먹기 때문이다.
// 3바이트 문자 중에서도 한글(U+AC00~U+D7A3)만 2칸이고 박스 문자는 1칸이라,
// 코드포인트를 계산해서 구분한다.
int display_width(const std::string& s);

// display_width 기준으로 공백을 채워 width 칸을 맞춘다.
// 이미 넘치면 자르지 않고 그대로 둔다 — 바이트 단위로 자르면 글자가 깨진다.
std::string pad(const std::string& s, int width, bool right_align = false);

// 앞뒤 공백(space, tab, CR, LF)을 떼어 낸다. C++ 표준에는 없는 기능이다.
std::string trim(const std::string& s);

// 소수점 아래 digits 자리로 고정한 문자열. std::to_string 은 자릿수를
// 제어할 수 없어서(항상 6자리) 표 출력에 쓸 수 없다.
std::string to_fixed(double value, int digits);

// ── 입력 ──────────────────────────────────────
//
// 모두 올바른 값이 들어올 때까지 되묻는다. EOF 면 InputCancelled 를 던진다.
// std::cin >> 를 쓰지 않고 getline 으로 한 줄씩 받는 이유는, >> 가 실패하면
// 잘못된 입력이 버퍼에 남아 무한 루프가 되기 때문이다.

double read_double(const std::string& label);

// 빈 줄을 입력하면 default_value 를 돌려주는 판. 화면에 [기본값] 이 표시된다.
double read_double(const std::string& label, double default_value);

int read_int(const std::string& label);
bool ask_yes_no(const std::string& label);

// ── 출력 ──────────────────────────────────────

void print_menu();

// 표 테두리 한 줄. 모서리 문자만 바꿔서 위/중간/아래를 모두 그린다.
//   print_table_line(W, "┌", "┬", "┐")   →   ┌────┬────┐
// widths 는 셀 내용의 폭이며, 좌우 여백 1칸씩은 이 함수가 더해 준다.
void print_table_line(const std::vector<int>& widths,
                      const std::string& left,
                      const std::string& mid,
                      const std::string& right);

// 표 내용 한 줄. cells 개수가 widths 와 다르면 아무것도 출력하지 않는다.
// 숫자 열은 right_align = true 로 오른쪽 정렬하는 편이 읽기 좋다.
void print_table_row(const std::vector<int>& widths,
                     const std::vector<std::string>& cells,
                     bool right_align = false);

// model.h — 실험 데이터를 담는 자료 구조
//
// 실험 1회 = Record 1개 (입력 변인 3개 + 측정한 출력 1개).
// DataSet 은 그 Record 들을 모아 두고, 회귀 모듈이 원하는 형태
// (설계 행렬 X, 응답 벡터 y)로 꺼내 준다.

#pragma once
#include <iostream>
#include <vector>
#include <array>
#include <string>
#include "linalg.h"

// 열 번호. ranges() 의 인덱스로 쓰인다.
// COLUMN_COUNT 를 맨 뒤에 두면 열 개수가 자동으로 따라온다.
// 앞의 3개가 입력 변인, RATE 가 출력이므로 "입력만" 돌 때는
// COLUMN_COUNT - 1 까지 순회한다.
enum Column
{
    TEMP,          // 온도 (°C)
    CATALYST,      // 촉매 질량 (g)
    H2O2,          // H2O2 초기 농도 (M)
    RATE,          // 산소 발생 속도 (mL/s) — 이것만 출력
    COLUMN_COUNT
};

// 실험 1회의 관측값.
struct Record
{
    double temperature;     // °C
    double catalyst_mass;   // g
    double h2o2_conc;       // M
    double o2_rate;         // mL/s

    // 입력 변인 3개만 {온도, 촉매, 농도} 순서로 반환. 회귀 입력에 쓴다.
    Vector factors() const;

    // 열 번호로 값을 꺼낸다. ranges() 처럼 네 열을 for 문으로 훑을 때 필요.
    double get(Column c) const;
};

std::ostream& operator<<(std::ostream& os, const Record& record);

// 한 변인의 관측 최솟값 / 최댓값.
struct Range
{
    double lo;
    double hi;
};

class DataSet
{
    // 유일한 상태. private 으로 숨겨서 아래 함수들로만 바뀌게 한다.
    std::vector<Record> records;

public:
    void add_record(const Record& record);
    int size() const;

    // 행 번호는 화면에 보이는 것과 같게 1-기반이다.
    // 범위를 벗어나면 std::out_of_range 를 던지므로 메뉴 층에서 잡는다.
    const Record& get_record(int index) const;
    void delete_record(int index);

    void clear();

    // 열별 최소/최대. 데이터가 없으면 전부 {0, 0}.
    // 요약 출력, 탐색 범위 기본값, 영향도 계산에 쓰인다.
    std::array<Range, COLUMN_COUNT> ranges() const;

    // 회귀용으로 데이터를 세로로 쪼갠다.
    // design_matrix() 는 절편용 1 열이 없는 n×3 이다. 1 열은 regression 이 붙인다.
    Matrix design_matrix() const;
    Vector response() const;      // o2_rate 만 모은 것
};

// optimizer.h — 격자 탐색으로 최적 조건 찾기
//
// 미분으로 최댓값을 구하는 대신, 탐색 범위를 잘게 나눠 모든 조합의 예측값을
// 계산해 두고 그중에서 고른다. 변인이 3개뿐이라 이 방식으로 충분하다.
//
// grid_search 는 "모든 후보를 계산해 돌려주기"만 한다. 무엇을 최적으로 볼지
// (속도 최대 / 목표에 근접 / 촉매 최소)는 부르는 쪽이 정렬로 정한다.
// 덕분에 판정 기준이 늘어도 이 함수는 고칠 일이 없다.

#pragma once
#include "regression.h"

struct Candidate
{
    Vector factors;   // {온도, 촉매, 농도}
    double rate;      // 그 조건에서의 예측 속도
};

// ranges 는 변인 3개의 탐색 구간. divisions 는 각 변인을 몇 등분할지.
// 반환 개수는 (divisions+1)³ 이다 — 20이면 9,261개, 50이면 132,651개.
// 세제곱으로 늘어나므로 부르는 쪽에서 상한을 두는 것이 좋다.
//
// 던질 수 있는 예외: std::invalid_argument (divisions < 1 또는 ranges 크기가 3이 아님)
std::vector<Candidate> grid_search(const Model& model,
                                   const std::vector<Range>& ranges,
                                   int divisions);

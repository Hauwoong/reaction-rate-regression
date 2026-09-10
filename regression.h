// regression.h — 다중선형회귀 (최소제곱법)
//
// 구하려는 식:
//   속도 = β0 + β1×온도 + β2×촉매 + β3×농도
//
// 실험이 n건이면 식이 n개, 미지수는 4개다. n > 4 면 모두 만족하는 답이 없으므로
// "오차 제곱합이 최소가 되는 β" 를 찾는다. 그 조건을 미분해서 정리하면
// 4×4 연립방정식(정규방정식)이 나온다:
//
//   (XᵀX) β = Xᵀy
//
// 즉 n개짜리 문제가 4×4 로 압축되고, 그걸 linalg::solve 가 푼다.

#pragma once
#include "model.h"

// 관측치가 계수 개수보다 적어 풀 수 없다.
struct NotEnoughData {};

// 모든 실험의 출력값이 같다(SST = 0). R² 를 정의할 수 없다.
struct ConstantResponse {};

struct Model
{
    Vector coefficients;   // β0(절편), β1, β2, β3 — factors 보다 1개 많다
    Vector fitted;         // 각 실험의 예측값 (0-기반, data 의 순서와 같음)
    Vector residuals;      // 실측 - 예측
    double r2 = 0.0;       // 결정계수. 1 - SSE/SST = "평균으로 찍는 것보다 얼마나 나은가"
    double rmse = 0.0;     // √(SSE/n). 평균적으로 몇 mL/s 틀리는지, 단위가 데이터와 같다

    // factors 는 {온도, 촉매, 농도} 3개. 절편은 coefficients[0] 이라
    // coefficients[i+1] 과 factors[i] 가 짝이 된다.
    double predict(const Vector& factors) const;
};

// 회귀를 수행한다.
// 던질 수 있는 예외: NotEnoughData, ConstantResponse, SingularMatrix
Model fit(const DataSet& data);

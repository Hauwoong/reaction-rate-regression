#include "regression.h"
#include "linalg.h"
#include <cmath>
#include <stdexcept>

// 각 행 앞에 1.0 을 붙여 n×3 을 n×4 로 만든다.
// 회귀식의 β0 에는 항상 1이 곱해지므로, 절편도 다른 계수와 똑같이 다루려면
// X 에 1로 채워진 열이 하나 필요하다.
static Matrix with_intercept(const Matrix& x)
{
    Matrix X;

    for (const auto& row : x)
    {
        Vector r = {1.0};
        r.insert(r.end(), row.begin(), row.end());
        X.push_back(r);
    }

    return X;
}

double Model::predict(const Vector& factors) const
{
    if (coefficients.size() != factors.size() + 1)
        throw std::invalid_argument("predict: 계수 개수와 변인 개수가 맞지 않습니다");

    double result = coefficients[0];

    for (size_t i = 0; i < factors.size(); ++i)
        result += coefficients[i+1] * factors[i];

    return result;
}

Model fit(const DataSet& data)
{
    // 미지수가 4개(절편 + 변인 3개)이므로 식도 최소 4개는 있어야 한다.
    // 딱 4건이면 오차가 0으로 나오지만 통계적 의미는 없다 — 실제로는 10건 이상 권장.
    if (data.size() < COLUMN_COUNT)
        throw NotEnoughData{};

    Matrix X = with_intercept(data.design_matrix());
    Vector y = data.response();

    Model m;

    // 정규방정식 (XᵀX)β = Xᵀy 를 세우고 푼다. 이 한 줄이 회귀의 전부다.
    m.coefficients = solve(xt_x(X), xt_y(X,y));

    // 구한 계수로 각 실험을 다시 예측해 잔차를 남긴다.
    // get_record 는 1-기반, fitted/residuals 는 0-기반 벡터임에 주의.
    for (int i = 1; i <= data.size(); ++i)
    {
        const Record& r = data.get_record(i);
        double p = m.predict(r.factors());

        m.fitted.push_back(p);
        m.residuals.push_back(r.o2_rate - p);   // 실측 - 예측 (순서 뒤집으면 부호가 반대)
    }

    // SSE = 모델이 못 맞춘 정도
    double sse = 0.0;
    for (double r : m.residuals)
        sse += r * r;

    m.rmse = std::sqrt(sse / m.residuals.size());

    double mean = 0.0;
    for (double r : y)
        mean += r;
    mean = mean / y.size();

    // SST = 아무 모델 없이 "그냥 평균으로 찍었을 때" 틀리는 정도
    double sst = 0.0;
    for (double v : y)
        sst += (v - mean) * (v - mean);

    // 모든 출력값이 같으면 SST 가 0이라 나눌 수 없다.
    // 소수 계산에서 정확히 0이 나오는 일은 드물어 작은 값과 비교한다.
    if (sst < 1e-12)
        throw ConstantResponse {};

    // R² = 1 - SSE/SST = "평균으로 찍는 것보다 얼마나 나은가"
    m.r2 = 1.0 - sse / sst;

    return m;
}
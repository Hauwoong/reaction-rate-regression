#include "regression.h"
#include "linalg.h"
#include <cmath>
#include <stdexcept>

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
    if (data.size() < COLUMN_COUNT)
        throw NotEnoughData{};

    Matrix X = with_intercept(data.design_matrix());
    Vector y = data.response();

    Model m;
    m.coefficients = solve(xt_x(X), xt_y(X,y));

    for (int i = 1; i <= data.size(); ++i)
    {
        const Record& r = data.get_record(i);
        double p = m.predict(r.factors());

        m.fitted.push_back(p);
        m.residuals.push_back(r.o2_rate - p);
    }

    double sse = 0.0;
    for (double r : m.residuals)
        sse += r * r;

    m.rmse = std::sqrt(sse / m.residuals.size());

    double mean = 0.0;
    for (double r : y)
        mean += r;
    mean = mean / y.size();

    double sst = 0.0;
    for (double v : y)
        sst += (v - mean) * (v - mean);

    if (sst < 1e-12)
        throw ConstantResponse {};

    m.r2 = 1.0 - sse / sst;

    return m;
}
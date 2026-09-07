#include <cmath>
#include <algorithm>
#include <stdexcept>
#include "linalg.h"

Matrix xt_x(const Matrix& x)
{
    if (x.size() == 0)
    {
        throw SingularMatrix{};
    }

    size_t p = x[0].size();
    Matrix result(p, Vector(p, 0.0));

    for (const auto& row : x)
        for (size_t i = 0; i < p; ++i)
            for (size_t j = 0; j < p; ++j)
                result[i][j] += row[i] * row[j];

    return result;
}

Vector xt_y(const Matrix& x, const Vector& y)
{
    if (x.size() != y.size())
    {
        throw std::invalid_argument("xt_y: X의 행 수와 y의 길이가 다릅니다");
    }

    if (x.size() == 0)
    {
        throw SingularMatrix{};
    }

    size_t p = x[0].size();
    Vector result(p, 0.0);

    for (size_t k = 0; k < x.size(); ++k)
        for (size_t i = 0; i < p; ++i)
            result[i] += x[k][i] * y[k];
    
    return result;
}

Vector solve(Matrix a, Vector b)
{
    size_t n = b.size();

    for (size_t col = 0; col < n; ++col)
    {
        size_t pivot = col;
        for (size_t r = col + 1; r < n; ++r)
            if (std::abs(a[r][col]) > std::abs(a[pivot][col]))
                pivot = r;

        std::swap(a[col], a[pivot]);
        std::swap(b[col], b[pivot]);

        if (std::abs(a[col][col]) < 1e-12)
            throw SingularMatrix{};

        for (size_t r = col + 1; r < n; ++r)
        {
            double factor = a[r][col] / a[col][col];

            for (size_t c = col; c < n; ++c)
                a[r][c] -= factor * a[col][c];

            b[r] -= factor * b[col];
        }
    }

    Vector x(n, 0.0);

    for (int r = static_cast<int>(n) -1; r >= 0; --r)
    {
        double sum = b[r];
        for (size_t c = r + 1; c < n; ++c)
            sum -= a[r][c] * x[c];

        x[r] = sum / a[r][r];
    }

    return x;
}
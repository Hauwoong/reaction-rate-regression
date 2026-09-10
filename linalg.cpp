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

// 가우스 소거법. 중학교에서 배우는 가감법을 컴퓨터가 따라 할 수 있게
// 순서를 정해 둔 것이다. 두 단계로 나뉜다.
//   1. 전진 소거 — 아래쪽 행에서 변수를 하나씩 없애 계단(위삼각) 모양을 만든다
//   2. 후진 대입 — 맨 아래 행부터 거꾸로 올라오며 값을 구한다
Vector solve(Matrix a, Vector b)
{
    size_t n = b.size();

    // ── 1. 전진 소거 ──
    for (size_t col = 0; col < n; ++col)
    {
        // 부분 피벗팅: col 열에서 절댓값이 가장 큰 행을 위로 올린다.
        // 바로 아래에서 이 값으로 나누기 때문에, 작은 수가 분모가 되면
        // 소수 계산 오차가 크게 증폭된다.
        size_t pivot = col;
        for (size_t r = col + 1; r < n; ++r)
            if (std::abs(a[r][col]) > std::abs(a[pivot][col]))
                pivot = r;

        // 행을 통째로 옮기는 것이므로 우변 b 도 같이 따라가야 한다.
        std::swap(a[col], a[pivot]);
        std::swap(b[col], b[pivot]);

        // 가장 큰 값조차 0에 가까우면 그 열은 다른 열에 종속이다 = 해가 없다.
        if (std::abs(a[col][col]) < 1e-12)
            throw SingularMatrix{};

        for (size_t r = col + 1; r < n; ++r)
        {
            // factor = "이 항을 0으로 만들려면 기준 행의 몇 배를 빼야 하는가"
            double factor = a[r][col] / a[col][col];

            // col 보다 왼쪽 열은 이전 단계에서 이미 0이라 계산할 필요가 없다
            for (size_t c = col; c < n; ++c)
                a[r][c] -= factor * a[col][c];

            b[r] -= factor * b[col];      // 좌변에 한 연산은 우변에도 똑같이
        }
    }

    // ── 2. 후진 대입 ──
    Vector x(n, 0.0);

    // 아래에서 위로 올라간다. size_t 는 부호가 없어 0에서 --하면 거대한 수가
    // 되므로(무한 루프) 여기서는 int 를 쓴다.
    for (int r = static_cast<int>(n) -1; r >= 0; --r)
    {
        double sum = b[r];

        // 이미 구한 값들을 우변으로 넘긴다
        for (size_t c = r + 1; c < n; ++c)
            sum -= a[r][c] * x[c];

        x[r] = sum / a[r][r];
    }

    return x;
}
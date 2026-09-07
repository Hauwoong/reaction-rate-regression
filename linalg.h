#pragma once
#include <vector>

struct SingularMatrix {};

using Vector = std::vector<double>;
using Matrix = std::vector<Vector>;

Matrix xt_x(const Matrix& x);
Vector xt_y(const Matrix& x, const Vector& y);
Vector solve(Matrix a, Vector b);
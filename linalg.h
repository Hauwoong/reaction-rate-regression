// linalg.h — 회귀에 필요한 최소한의 선형대수
//
// 이 파일은 model / storage / ui 를 전혀 모른다. 순수한 수학 도구이므로
// 다른 프로그램에 그대로 가져다 써도 된다. 의존 방향은 항상 아래(범용)에서
// 위(전용)로만 흐른다.

#pragma once
#include <vector>

// 해가 없는 연립방정식. 실험에서 변인 하나를 고정했거나 두 변인이 항상
// 함께 움직였을 때 발생한다. 사용자가 실험 설계를 고쳐야 하는 문제다.
struct SingularMatrix {};

using Vector = std::vector<double>;
using Matrix = std::vector<Vector>;   // Matrix[행][열]

// XᵀX 를 만든다. 결과는 p×p (p = x의 열 개수).
//   XᵀX[i][j] = Σ_k  X[k][i] × X[k][j]
// 정규방정식의 좌변(계수 행렬)에 해당한다.
Matrix xt_x(const Matrix& x);

// Xᵀy 를 만든다. 결과는 길이 p.
//   Xᵀy[i] = Σ_k  X[k][i] × y[k]
// 정규방정식의 우변에 해당한다.
Vector xt_y(const Matrix& x, const Vector& y);

// 연립방정식 a·x = b 를 풀어 x 를 반환한다 (부분 피벗팅 가우스 소거법).
//
// a, b 를 값으로 받는 것은 의도적이다. 소거 과정에서 행렬을 계속 고쳐야 하는데,
// 값으로 받으면 컴파일러가 복사본을 만들어 주므로 원본이 안전하다.
Vector solve(Matrix a, Vector b);

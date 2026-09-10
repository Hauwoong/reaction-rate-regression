#!/bin/bash
# macOS / Linux 빌드 스크립트
#
#   chmod +x build.sh     (처음 한 번만)
#   ./build.sh
#   ./co2
#
# macOS 에서 clang++ 를 쓰려면 아래 CXX 를 clang++ 로 바꾸면 된다.
# (Xcode Command Line Tools 를 설치하면 g++ 가 clang++ 의 별칭이라 그대로도 동작한다.)

set -e   # 컴파일 실패하면 즉시 중단

CXX=${CXX:-g++}
SRC="main.cpp model.cpp storage.cpp ui.cpp linalg.cpp regression.cpp optimizer.cpp"
OUT="co2"

echo "빌드 중... ($CXX)"
$CXX -std=c++20 -O2 -Wall -Wextra $SRC -o $OUT

echo "완료: ./$OUT"

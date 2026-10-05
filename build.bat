@echo off
REM Windows 빌드 스크립트 (컴파일러가 이미 있을 때)
REM 처음이라면 setup.bat 을 쓰세요 — 컴파일러 설치까지 해 줍니다.

chcp 65001 > nul

where g++ > nul 2>&1
if %errorlevel% neq 0 (
    echo g++ 컴파일러를 찾을 수 없습니다.
    echo setup.bat 을 실행하면 설치부터 해 줍니다.
    exit /b 1
)

g++ -std=c++17 -O2 -Wall main.cpp model.cpp storage.cpp ui.cpp linalg.cpp regression.cpp optimizer.cpp -o co2.exe

if %errorlevel% neq 0 (
    echo.
    echo 빌드 실패
    exit /b %errorlevel%
)

echo 완료: co2.exe

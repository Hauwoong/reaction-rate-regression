@echo off
REM Windows 빌드 스크립트
REM
REM   build.bat
REM   co2.exe

g++ -std=c++20 -O2 -Wall -Wextra main.cpp model.cpp storage.cpp ui.cpp linalg.cpp regression.cpp optimizer.cpp -o co2.exe

if %errorlevel% neq 0 (
    echo.
    echo 빌드 실패
    exit /b %errorlevel%
)

echo 완료: co2.exe

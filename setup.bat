@echo off
REM 처음 받은 컴퓨터에서 이 파일을 더블클릭하세요.
REM 컴파일러 확인/설치 후 co2.exe 를 만듭니다.
REM PowerShell 은 기본 설정상 .ps1 실행을 막아서 여기서 우회해 부릅니다.

powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0setup.ps1"

REM 더블클릭으로 열었을 때 창이 바로 닫히지 않도록
pause

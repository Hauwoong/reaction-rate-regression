# setup.ps1 — 처음 받은 컴퓨터에서 프로그램을 쓸 수 있게 준비한다.
#
#   1. C++ 컴파일러(g++ 9 이상)가 있는지 확인
#   2. 없으면 사용자 동의를 받고 winget 으로 설치
#   3. 프로그램 빌드 → co2.exe
#
# 직접 실행하지 말고 setup.bat 을 더블클릭하면 된다.
# (PowerShell 은 기본 설정상 .ps1 실행을 막기 때문에 setup.bat 이 우회해서 부른다.)

$ErrorActionPreference = 'Stop'
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
Set-Location $PSScriptRoot

# std::filesystem 이 안정적으로 들어간 것이 GCC 9 부터다.
$MinGccMajor = 9
$WingetId    = 'BrechtSanders.WinLibs.POSIX.UCRT'
$Sources     = 'main.cpp','model.cpp','storage.cpp','ui.cpp','linalg.cpp','regression.cpp','optimizer.cpp'

function Write-Step($text) { Write-Host ""; Write-Host "== $text ==" -ForegroundColor Cyan }
function Write-Ok($text)   { Write-Host "  [OK] $text" -ForegroundColor Green }
function Write-Warn($text) { Write-Host "  [!]  $text" -ForegroundColor Yellow }
function Write-Fail($text) { Write-Host "  [X]  $text" -ForegroundColor Red }

# winget 이 설치하면서 PATH 를 바꿔도 지금 열려 있는 창에는 반영되지 않는다.
# 레지스트리에서 다시 읽어 와 새 창을 연 것과 같은 효과를 낸다.
function Update-SessionPath {
    $machine = [Environment]::GetEnvironmentVariable('Path', 'Machine')
    $user    = [Environment]::GetEnvironmentVariable('Path', 'User')
    $env:Path = "$machine;$user"
}

# g++ 가 있으면 주 버전 번호를, 없으면 $null 을 돌려준다.
function Get-GccMajor {
    $cmd = Get-Command g++ -ErrorAction SilentlyContinue
    if (-not $cmd) { return $null }
    try {
        $ver = (& g++ -dumpversion 2>$null | Select-Object -First 1).Trim()
        return [int]($ver.Split('.')[0])
    } catch {
        return $null
    }
}

Write-Host ""
Write-Host "  H2O2 분해 속도 예측 프로그램 — 설치 도우미" -ForegroundColor White

# ─────────────────────────────────────────────
Write-Step "1/2  C++ 컴파일러 확인"
# ─────────────────────────────────────────────
$major = Get-GccMajor

if ($major -ge $MinGccMajor) {
    Write-Ok ("g++ {0} 이(가) 설치되어 있습니다: {1}" -f $major, (Get-Command g++).Source)
}
else {
    if ($null -eq $major) {
        Write-Warn "g++ 를 찾을 수 없습니다."
    } else {
        Write-Warn ("g++ {0} 이(가) 있지만 너무 오래되었습니다 (g++ {1} 이상 필요)." -f $major, $MinGccMajor)
        Write-Warn "Dev-C++ 에 딸린 옛 컴파일러일 가능성이 큽니다."
    }

    if (-not (Get-Command winget -ErrorAction SilentlyContinue)) {
        Write-Fail "winget 이 없어 자동으로 설치할 수 없습니다."
        Write-Host  "       Microsoft Store 에서 '앱 설치 관리자'를 설치/업데이트한 뒤 다시 실행하거나,"
        Write-Host  "       https://winlibs.com 에서 GCC 를 받아 bin 폴더를 PATH 에 추가하세요."
        exit 1
    }

    Write-Host ""
    Write-Host "  winget 으로 GCC 컴파일러(WinLibs, 약 250MB)를 설치합니다."
    $answer = Read-Host "  설치할까요? (y/n)"
    if ($answer -notmatch '^[yY]') {
        Write-Fail "설치를 취소했습니다. 컴파일러 없이는 빌드할 수 없습니다."
        exit 1
    }

    winget install --id $WingetId -e --accept-source-agreements --accept-package-agreements
    Update-SessionPath

    $major = Get-GccMajor
    if ($major -ge $MinGccMajor) {
        Write-Ok ("g++ {0} 설치 완료" -f $major)
    } else {
        Write-Fail "설치는 끝났지만 이 창에서 g++ 가 아직 보이지 않습니다."
        Write-Host  "       창을 닫고 setup.bat 을 다시 실행하세요."
        exit 1
    }
}

# ─────────────────────────────────────────────
Write-Step "2/2  프로그램 빌드"
# ─────────────────────────────────────────────
foreach ($f in $Sources) {
    if (-not (Test-Path $f)) {
        Write-Fail "$f 가 없습니다. 저장소를 통째로 받았는지 확인하세요."
        exit 1
    }
}

# C++17 이면 충분하다 (std::filesystem 이 최신 기능 중 유일하게 쓰인다).
& g++ -std=c++17 -O2 -Wall @Sources -o co2.exe
if ($LASTEXITCODE -ne 0) {
    Write-Fail "빌드에 실패했습니다. 위 에러의 첫 줄을 확인하세요."
    exit 1
}
Write-Ok "co2.exe 를 만들었습니다."

Write-Host ""
Write-Host "  준비 완료! 이 폴더에서 아래처럼 실행하세요:" -ForegroundColor Green
Write-Host "      .\co2.exe"
Write-Host ""
Write-Host "  참고: 그래프(visualize.m)는 MATLAB 이 있어야 그릴 수 있습니다."
Write-Host "        MATLAB 은 유료라 자동 설치하지 않으며, 프로그램 실행에는 필요 없습니다."

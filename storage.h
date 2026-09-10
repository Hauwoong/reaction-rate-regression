// storage.h — CSV 파일 입출력
//
// CSV 열 이름은 공백 없는 영문으로 고정한다. MATLAB readtable 이 열 이름을
// 변수명으로 쓰는데, 공백이나 한글이 들어가면 이름을 멋대로 바꿔 버리기 때문이다.
//
//   temperature_C,catalyst_g,h2o2_M,o2_rate_mL_s

#pragma once
#include <vector>
#include <string>
#include "model.h"

// list_csv_files 의 결과 한 줄.
struct FileInfo
{
    std::string name;   // 파일 이름만 (경로 없음)
    int count;          // 레코드 수. 읽기에 실패하면 -1
};

// data/, output/ 폴더가 없으면 만든다. 이미 있으면 아무 일도 안 한다.
// 프로그램 시작 시 한 번만 부르면 된다.
void ensure_directories();

// DataSet 을 CSV 로 저장한다. 경로에 .csv 가 없으면 붙여 준다.
// 던질 수 있는 예외: std::runtime_error (파일 열기 실패)
void save_csv(const DataSet& data, const std::string& path);

// CSV 를 읽어 DataSet 으로 돌려준다.
// 헤더가 있든 없든 알아서 처리한다(첫 줄의 첫 칸이 숫자가 아니면 헤더로 본다).
// 빈 줄은 건너뛰고, 잘못된 줄을 만나면 줄 번호를 붙여 예외를 던진다.
// 던질 수 있는 예외: std::runtime_error
DataSet load_csv(const std::string& path);

// 폴더 안의 .csv 파일 목록을 이름순으로 돌려준다.
// 읽을 수 없는 파일이 섞여 있어도 목록 전체가 실패하지 않고 count 가 -1 이 된다.
std::vector<FileInfo> list_csv_files(const std::string& directory);

// .csv 로 끝나지 않으면 붙여서 돌려준다. 대소문자는 구분하지 않는다(.CSV 도 인정).
std::string with_csv_extension(const std::string& filename);

// 임의의 표를 CSV 로 쓴다. 값은 전부 문자열로 미리 바꿔서 넘긴다.
// DataSet 이 아닌 것(회귀 계수, 예측값, 격자 등)을 내보낼 때 쓴다.
//
// 모든 행의 칸 수가 header 와 같아야 하며, 검사는 파일을 열기 전에 끝낸다.
// 그래야 실패했을 때 반쪽짜리 파일이 디스크에 남지 않는다.
// 던질 수 있는 예외: std::invalid_argument (칸 수 불일치), std::runtime_error (열기 실패)
void write_csv(const std::string& path,
               const std::vector<std::string>& header,
               const std::vector<std::vector<std::string>>& rows);

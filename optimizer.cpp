#include "optimizer.h"
#include <stdexcept>

std::vector<Candidate> grid_search(const Model& model, 
    const std::vector<Range>& bounds, 
    int divisions)
{
    if (divisions < 1 || bounds.size() != 3)
        throw std::invalid_argument("grid_search: 분할 수 또는 범위가 잘못되었습니다");

    // 변인마다 스케일이 다르므로(온도는 20단위, 촉매는 0.25단위) 간격을 하나로
    // 통일할 수 없다. 대신 "몇 등분할지"를 받아 변인별 간격을 따로 계산한다.
    double step0 = (bounds[0].hi - bounds[0].lo) / divisions;
    double step1 = (bounds[1].hi - bounds[1].lo) / divisions;
    double step2 = (bounds[2].hi - bounds[2].lo) / divisions;

    std::vector<Candidate> result;

    // <= 인 이유: divisions 등분하면 눈금(점)은 divisions+1 개다.
    // 값을 lo + i*step 으로 매번 새로 계산하는 것도 의도적이다.
    // t += step 으로 누적하면 소수 오차가 쌓여 마지막 값이 hi 에 정확히 닿지 않는다.
    for (int i = 0; i <= divisions; ++i)
    {
        // 각 값은 자기 루프 바로 아래에서 계산한다.
        // 안쪽에 두면 바뀌지도 않는 값을 수천 번 다시 계산하게 된다.
        double t = bounds[0].lo + i * step0;

        for (int j = 0; j <= divisions; ++j)
        {
            double c = bounds[1].lo + j * step1;

            for (int k = 0; k <= divisions; ++k)
            {
                double h = bounds[2].lo + k * step2;

                Vector factors = {t, c, h};
                result.push_back({factors, model.predict(factors)});
            }
        }
    }

    return result;
}
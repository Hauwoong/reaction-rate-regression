#include "optimizer.h"
#include <stdexcept>

std::vector<Candidate> grid_search(const Model& model, 
    const std::vector<Range>& bounds, 
    int divisions)
{
    if (divisions < 1 || bounds.size() != 3)
        throw std::invalid_argument("grid_search: 분할 수 또는 범위가 잘못되었습니다");

    double step0 = (bounds[0].hi - bounds[0].lo) / divisions;
    double step1 = (bounds[1].hi - bounds[1].lo) / divisions;
    double step2 = (bounds[2].hi - bounds[2].lo) / divisions;

    std::vector<Candidate> result;

    for (int i = 0; i <= divisions; ++i)
    {
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
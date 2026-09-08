#pragma once
#include "regression.h"

struct Candidate
{
    Vector factors;
    double rate;
};

std::vector<Candidate> grid_search(const Model& model, 
    const std::vector<Range>& ranges, 
    int divisions);
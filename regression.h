#pragma once
#include "model.h"

struct NotEnoughData {};
struct ConstantResponse {};

struct Model
{
    Vector coefficients;
    Vector fitted;
    Vector residuals;
    double r2 = 0.0;
    double rmse = 0.0;

    double predict(const Vector& factors) const;
};

Model fit(const DataSet& data);

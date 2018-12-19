#pragma once
#include <vector>

class algorithm
{
public:
    virtual ~algorithm() = default;

    virtual void preprocessing(const std::vector<float>& points) = 0;
    virtual std::vector<float> get_visibility(double x, double y) = 0;
};
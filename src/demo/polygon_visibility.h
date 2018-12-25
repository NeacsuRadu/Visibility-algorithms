#pragma once
#include "algorithm.h"
#include "triangulation.h"

class polygon_visibility: 
    public algorithm
{
public:
    void preprocessing(const std::vector<float>& points) override;
    std::vector<float> get_visibility(float x, float y) override;
private:
    
    std::vector<triangle*> m_triangles;
};
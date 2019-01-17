#include "polygon_helpers.h"

#include "geometry.h"

bool is_counter_clockwise(const std::vector<point>& points)
{
    double sum = 0;
    auto sz = points.size();
    for (std::size_t idx = 0; idx < sz - 1; ++idx)
    {
        auto det = determinant(points[idx].x, points[idx + 1].x, points[idx].y, points[idx + 1].y);
        sum += det;
    }

    auto det = determinant(points[sz - 1].x, points[0].x, points[sz - 1].y, points[0].y);
    sum += det;

    if (sum < 0.0)
    {
        //std::cout << "is not counter clockwise" << std::endl;
        return false;
    }
    //std::cout << "is counter clockwise" << std::endl;
    return true;
}

void revert_order(std::vector<point>& points)
{
    std::vector<point> aux(points.rbegin(), points.rend());
    points.swap(aux);
}


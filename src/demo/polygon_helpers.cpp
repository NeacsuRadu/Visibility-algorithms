#include "polygon_helpers.h"

#include "geometry.h"

bool is_counter_clockwise(const std::vector<point>& points)
{
    int sum = 0;
    for (std::size_t idx = 0; idx < points.size() - 2; ++idx)
    {
        auto orient = test_orientation(points[idx], points[idx + 1], points[idx +2]);
        sum += static_cast<int>(orient);
    }

    if (sum < 0)
        return false;
    return true;
}

void revert_order(std::vector<point>& points)
{
    std::vector<point> aux(points.rbegin(), points.rend());
    points.swap(aux);
}


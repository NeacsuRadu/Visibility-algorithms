#include "intersection_visibility.h"

#include <algorithm>

intersect_visibility * singleton<intersect_visibility>::instance = nullptr;

std::vector<triangle*> intersect_visibility::get_visibility(const point& view)
{
    if (!point_in_polygon(m_polygons[0], view))
        return {};
    
    for (std::size_t i = 1; i < m_polygons.size(); ++i)
        if (point_in_polygon(m_polygons[i], view))
            return {};

    auto aux = m_polygons;
    for (auto& polygon: aux)
    {
        for (auto& pt: polygon)
        {
            pt.x -= view.x;
            pt.y -= view.y;
        }
    }

    std::vector<point> angles;
    for (const auto& polygon: aux)
        angles.insert(angles.end(), polygon.begin(), polygon.end());

    std::sort(angles.begin(), angles.end(), [](const point& p1, const point& p2) -> bool {
        auto dist_p1 = std::sqrt(std::pow(p1.x, 2.0) + std::pow(p1.y, 2.0));
        auto angle_p1 = angleo(p1);

        auto dist_p2 = std::sqrt(std::pow(p2.x, 2.0) + std::pow(p2.y, 2.0));
        auto angle_p2 = angleo(p2);

        if (angle_p1 == angle_p2)
            return dist_p1 < dist_p2;
        return angle_p1 < angle_p2;
    });

    std::vector<point> visible_points;
    for (auto pt: angles)
    {
        double d = 1000.0;
        auto ang = angleo(pt);

        auto pre_ang = ang - 0.00001;
        point pre_pt;
        pre_pt.y = std::sin(pre_ang) * d + view.y;
        pre_pt.x = std::cos(pre_ang) * d + view.x;
        auto pre_visible_point = _get_visible_point(view, pre_pt);


        pt.x += view.x;
        pt.y += view.y;
        auto visible_point = _get_visible_point(view, pt);

        auto post_ang = ang + 0.00001;
        point post_pt;
        post_pt.y = std::sin(post_ang) * d + view.y;
        post_pt.x = std::cos(post_ang) * d + view.x;
        auto post_visible_point = _get_visible_point(view, post_pt);

        visible_points.push_back(pre_visible_point);
        visible_points.push_back(visible_point);
        visible_points.push_back(post_visible_point);
    }

    std::vector<triangle*> res;
    for (std::size_t i = 0; i < visible_points.size() - 1; ++ i)
        res.push_back(get_triangle(view, visible_points[i], visible_points[i + 1]));
    res.push_back(get_triangle(view, visible_points[visible_points.size() - 1], visible_points[0]));

    return res;
}

point intersect_visibility::_get_visible_point(const point& view, const point& pt)
{
    auto intersection = pt;
    auto dist = distance(view, pt);
    for (std::size_t i = 0; i < m_polygons.size(); ++ i)
    {
        for (std::size_t j = 0; j < m_polygons[i].size() - 1; ++ j)
        {
            auto inte = get_segments_intersection(view, pt, m_polygons[i][j], m_polygons[i][j + 1]);
            if (inte == error_point)
                continue;


            auto d = distance(view, inte);
            if (d < dist)
            {
                dist = d;
                intersection = inte;
            }
        }

        auto inte = get_segments_intersection(view, pt, m_polygons[i][m_polygons[i].size() - 1], m_polygons[i][0]);
        if (inte != error_point)
        {
            auto d = distance(view, inte);
            if (d < dist)
            {
                dist = d;
                intersection = inte;
            }
        }
    }

    return intersection;
}
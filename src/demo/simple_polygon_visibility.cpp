#include "simple_polygon_visibility.h"
#include "polygon_helpers.h"

simple_polygon_visibility * singleton<simple_polygon_visibility>::instance = nullptr;

void simple_polygon_visibility::preprocess_polygons(const std::vector<std::vector<point>>& polygons)
{
    m_polygon = polygons[0];
    if (!is_counter_clockwise(m_polygon))
        revert_order(m_polygon);
}

std::vector<triangle*> simple_polygon_visibility::get_visibility(const point& view)
{
    if (!point_in_polygon(m_polygon, view))
        return {};

    _shift_polygon_points(view);
    auto st = _get_visibility_stack(view);
    std::vector<triangle*> result;
    point crr = st.top().pt;
    point last = crr;
    st.pop();
    while (!st.empty())
    {
        std::cout << "point: " << crr.x << " " << crr.y << std::endl;
        result.push_back(get_triangle(view, st.top().pt, crr));
        crr = st.top().pt;
        st.pop();
    }
    result.push_back(get_triangle(view, last, crr));
    return result;
}

void simple_polygon_visibility::_shift_polygon_points(const point& view)
{
    double min = 1000000.0;
    std::size_t idx_min = 0;
    point paux { 1000.0, view.y };
    m_polygon.push_back(m_polygon[0]);
    for (std::size_t idx = 0; idx < m_polygon.size() - 1; ++idx)
    {
        point intersect = get_segments_intersection(m_polygon[idx], m_polygon[idx + 1], view, paux);
        if (intersect == error_point)
            continue;

        double dist = distance(view, intersect);
        if (dist < min)
        {
            min = dist;
            idx_min = idx + 1;
        }
    }

    m_polygon.pop_back();
    if (idx_min == m_polygon.size())
        return;

    std::vector<point> aux;
    for (std::size_t idx = idx_min; idx < m_polygon.size(); ++idx)
        aux.push_back(m_polygon[idx]);
    for (std::size_t idx = 0; idx < idx_min; ++idx)
        aux.push_back(m_polygon[idx]);
    m_polygon.swap(aux);
}

std::stack<simple_polygon_visibility::stack_data> simple_polygon_visibility::_get_visibility_stack(const point& view)
{
    int idx = 1;
    int sz = m_polygon.size();
    std::stack<stack_data> st;
    st.push({m_polygon[0], true, 0});
    while (idx < sz)
    {
        if (test_orientation(view, m_polygon[index(idx - 1, sz)], m_polygon[index(idx, sz)]) != orientation::right)
        {
            std::cout << "vertex is to the left of the previous one, push" << std::endl;
            st.push({m_polygon[index(idx, sz)], true, idx});
            idx ++;
            continue;
        }

        std::cout << "vertex is to the right of the previous one" << std::endl;
        if (test_orientation(m_polygon[index(idx - 2, sz)], m_polygon[index(idx - 1, sz)], m_polygon[index(idx, sz)]) == orientation::right)
        {
            std::cout << "vertex is to the right of the -2 -1 line" << std::endl;
            point curr = m_polygon[index(idx - 1, sz)];
            idx ++;
            while (idx <= sz)
            {
                auto intersect = get_lines_intersection(view, curr, m_polygon[index(idx - 1, sz)], m_polygon[index(idx, sz)]);
                if (intersect != error_point && 
                    point_between_segment_vertices(intersect, m_polygon[index(idx - 1, sz)], m_polygon[index(idx, sz)]))
                {
                    st.push({intersect, false, idx - 1});
                    st.push({m_polygon[index(idx, sz)], true, idx});
                    break;
                }
                idx ++;
            }
            idx ++;
        }
        else 
        {
            std::cout << "vertex is to the left of -2 -1 line" << std::endl;
            point f1 = m_polygon[index(idx - 1, sz)]; // forward edge
            point f2 = m_polygon[index(idx, sz)];
            while (!st.empty())
            {
                auto data = st.top();
                auto intersection = get_segments_intersection(f1, f2, view, data.pt);
                if (intersection == error_point)
                {
                    auto ori = test_orientation(view, m_polygon[index(idx, sz)], m_polygon[index(idx + 1, sz)]);
                    if (ori == orientation::right)
                    {
                        std::cout << "idx + 1 is to the right of q idx " << std::endl;
                        f1 = f2;
                        f2 = m_polygon[index(idx + 1, sz)];
                        idx ++;
                        continue;
                    }
                    else 
                    {
                        if (test_orientation(f1, f2, m_polygon[index(idx + 1, sz)]) == orientation::right)
                        {
                            std::cout << "idx + 1 is to the right of idx - 1 idx " << std::endl;
                            auto m = get_lines_intersection(view, f2, data.pt, m_polygon[index(data.index + 1, sz)]);
                            if (m == error_point)
                                throw std::runtime_error("lines do not intersect, error");

                            st.push({m, false, data.index});
                            st.push({f2, true, idx});
                            idx ++;
                            break;
                        }
                        else 
                        {
                            std::cout << "idx + 1 is to the left of idx - 1 idx " << std::endl;
                            idx ++;
                            while (idx < sz)
                            {
                                //std::cout << "while" << std::endl;
                                intersection = get_lines_intersection(view, f2, m_polygon[index(idx, sz)], m_polygon[index(idx + 1, sz)]);
                                if (intersection != error_point &&
                                    point_between_segment_vertices(intersection, m_polygon[index(idx, sz)], m_polygon[index(idx + 1, sz)]))
                                {
                                    f2 = m_polygon[index(idx + 1, sz)];
                                    f1 = m_polygon[index(idx, sz)];
                                    idx ++;
                                    break;
                                }
                                idx ++;
                            }
                        }
                    }
                }
                else
                {
                    if (data.is_vertex)
                    {
                        //std::cout << "stack popped" << std::endl;
                        st.pop();
                    }
                    else
                    {
                        st.pop();
                        if (point_between_segment_vertices(intersection, data.pt, st.top().pt))
                        {
                            idx ++;
                            while (idx < sz)
                            {
                                point z = get_lines_intersection(intersection, st.top().pt, m_polygon[index(idx, sz)], m_polygon[index(idx - 1, sz)]);
                                if (z != error_point &&
                                    point_between_segment_vertices(z, m_polygon[index(idx, sz)], m_polygon[index(idx - 1, sz)]))
                                {
                                    st.push({z, false, idx - 1});
                                    st.push({m_polygon[index(idx, sz)], true, idx});
                                    idx++;
                                    break;
                                }
                                idx ++;
                            }
                            break;
                        }
                    }
                }
            }
        }
    }

    return st;
}
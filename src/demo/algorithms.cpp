#include "algorithms.h"
#include "geometry.h"
#include "circular_list.h"

#include <stack>
#include <iostream>

struct stack_data 
{
    point pt;
    bool is_vertex = false;
    int index = -1;
};

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

void to_counter_clockwise(std::vector<point>& points)
{
    std::vector<point> aux(points.rbegin(), points.rend());
    points.swap(aux);
}

void simple_polygon_preprocessing(std::vector<point>& points, const point& origin)
{
    if (!is_counter_clockwise(points))
        to_counter_clockwise(points);

    float min = 2.0;
    std::size_t idx_min = 0;
    point paux { 1.0, origin.y };
    points.push_back(points[0]);
    for (std::size_t idx = 0; idx < points.size() - 1; ++idx)
    {
        point intersect = get_segments_intersection(points[idx], points[idx + 1], origin, paux);
        if (intersect == error_point)
            continue;

        float dist = distance(origin, intersect);
        if (dist < min)
        {
            min = dist;
            idx_min = idx + 1;
        }
    }

    points.pop_back();
    if (idx_min == points.size())
        return;

    std::vector<point> aux;
    for (std::size_t idx = idx_min; idx < points.size(); ++idx)
        aux.push_back(points[idx]);
    for (std::size_t idx = 0; idx < idx_min; ++idx)
        aux.push_back(points[idx]);
    points.swap(aux);
}

int index(int idx, int size)
{
    if (idx >= 0)
        return idx % size;
    return size + (idx % size);
}

std::vector<float> get_simple_polygon_visibility(const std::vector<float>& points, float px, float py)
{
    std::vector<point> vertices;
    for (auto it = points.begin(); it != points.end(); ++it)
    {   
        float x = *it;
        float y = *(++it);
        vertices.push_back({x, y});
    }

    if (!point_in_polygon(vertices, {px, py}))
        return {};

    point q {px, py};
    simple_polygon_preprocessing(vertices, q);

   
    int idx = 1;
    int sz = vertices.size();
    std::stack<stack_data> st;
    st.push({vertices[0], true, 0});
    while (idx < vertices.size())
    {
        if (test_orientation(q, vertices[index(idx - 1, sz)], vertices[index(idx, sz)]) != orientation::right)
        {
            std::cout << "vertex is to the left of the previous one, push" << std::endl;
            st.push({vertices[index(idx, sz)], true, idx});
            idx ++;
            continue;
        }

        std::cout << "vertex is to the right of the previous one" << std::endl;
        if (test_orientation(vertices[index(idx - 2, sz)], vertices[index(idx - 1, sz)], vertices[index(idx, sz)]) == orientation::right)
        {
            std::cout << "vertex is to the right of the -2 -1 line" << std::endl;
            point curr = vertices[index(idx - 1, sz)];
            idx ++;
            while (idx < sz)
            {
                auto intersect = get_lines_intersection(q, curr, vertices[index(idx - 1, sz)], vertices[index(idx, sz)]);
                if (intersect != error_point && 
                    point_between_segment_vertices(intersect, vertices[index(idx - 1, sz)], vertices[index(idx, sz)]))
                {
                    st.push({intersect, false});
                    st.push({vertices[index(idx, sz)], true, idx});
                    break;
                }
                idx ++;
            }
            idx ++;
        }
        else 
        {
            std::cout << "vertex is to the left of -2 -1 line" << std::endl;
            point f1 = vertices[index(idx - 1, sz)];
            point f2 = vertices[index(idx, sz)];
            st.pop();
            while (!st.empty())
            {
                if (!st.top().is_vertex)
                {
                    // presupunem ca se intersecteaza :) refacem mai tarziu algoritmul 
                    std::cout << "point is not vertex" << std::endl;
                    point u = st.top().pt;
                    st.pop();
                    
                    point intersection = get_segments_intersection(f1, f2, q, u);
                    if (intersection == error_point)
                        throw std::runtime_error("lines do not intersect, cannot continue");

                    if (!point_between_segment_vertices(intersection, st.top().pt, u))
                    {
                        // atunci ramane ca pana acum si continuam cu w
                        continue;
                    }
                    else
                    {
                        // trebuie sa cautam urmatorul punct vizibil
                        idx ++;
                        while (idx < sz)
                        {
                            point z = get_segments_intersection(intersection, st.top().pt, vertices[index(idx, sz)], vertices[index(idx - 1, sz)]);
                            if (z != error_point)
                            {
                                st.push({z, false, -1});
                                st.push({vertices[index(idx, sz)], true, idx});
                                idx++;
                                break;
                            }
                            idx ++;
                        }
                    }
                }

                point intersection = get_segments_intersection(f1, f2, q, st.top().pt);
                if (intersection != error_point)
                {
                    std::cout << "segments intersect - pop" << std::endl;
                    st.pop();
                    continue;
                }

                auto orientation = test_orientation(q, vertices[index(idx, sz)], vertices[index(idx + 1, sz)]);
                if (orientation != orientation::right)
                {
                    std::cout << "+1 is to the left of i" << std::endl;
                    if (test_orientation(vertices[index(idx - 1, sz)], vertices[index(idx, sz)], vertices[index(idx + 1, sz)]) == orientation::right)
                    {
                        std::cout << "+1 is to the right of -1 i" << std::endl;
                        intersection = get_lines_intersection(q, f2, st.top().pt, vertices[index(st.top().index + 1, sz)]);
                        if (intersection == error_point)
                            throw std::runtime_error("error, lines do not intersect");

                        st.push({intersection, false, -1});
                        st.push({vertices[index(idx, sz)], true, idx});
                        idx ++;
                        break;
                    }
                    else 
                    {
                        std::cout << "+1 is to the left of -1 i" << std::endl;
                        idx ++;
                        while (idx < sz)
                        {
                            std::cout << "while" << std::endl;
                            intersection = get_lines_intersection(q, f2, vertices[index(idx, sz)], vertices[index(idx + 1, sz)]);
                            if (intersection != error_point &&
                                point_between_segment_vertices(intersection, vertices[index(idx, sz)], vertices[index(idx + 1, sz)]))
                            {
                                f2 = vertices[index(idx + 1, sz)];
                                f1 = vertices[index(idx, sz)];
                                idx ++;
                                break;
                            }
                            idx ++;
                        }
                    }
                }
                else if (orientation == orientation::right)
                {
                    std::cout << "+1 is to the right of i, continue with new edge" << std::endl;
                    f1 = f2;
                    f2 = vertices[index(idx + 1, sz)];
                    idx ++;
                    continue;
                }
            }
        }
    }

    std::vector<float> result;
    while (!st.empty())
    {
        result.push_back(st.top().pt.x);
        result.push_back(st.top().pt.y);
        st.pop();
    }
    return result;
}
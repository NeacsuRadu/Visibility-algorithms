#include "algorithms.h"
#include "geometry.h"
#include "triangulation.h"

#include <stack>
#include <iostream>
#include <algorithm>

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

void revert_order(std::vector<point>& points)
{
    std::vector<point> aux(points.rbegin(), points.rend());
    points.swap(aux);
}

void simple_polygon_preprocessing(std::vector<point>& points, const point& origin)
{
    if (!is_counter_clockwise(points))
        revert_order(points);

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

struct bridge_edge 
{
    std::size_t index_polygon;
    std::size_t index_hole;
    double distance = 0.0;
};

std::vector<triangle*> triangles;
std::vector<point> get_degenerate_polygon(std::vector<std::vector<point>>& polygons)
{
    if (polygons.size() == 1)
        return polygons[0];

    std::vector<bridge_edge> bridges;
    for (std::size_t idx_polygon = 0; idx_polygon < polygons[0].size(); ++ idx_polygon)
        for (std::size_t idx_hole = 0; idx_hole < polygons[1].size(); ++ idx_hole)
            bridges.push_back({idx_polygon, idx_hole, distance(polygons[0][idx_polygon], polygons[1][idx_hole])});

    std::sort(bridges.begin(), bridges.end(), [](const bridge_edge& e1, const bridge_edge& e2) { 
        return e1.distance <= e2.distance; });

    std::size_t edge_index = 0;
    for (const auto& e : bridges)
    {
        bool is_valid = true;
        for (std::size_t idx = 0; idx < polygons[0].size() - 1; ++ idx)
        {
            if (polygons[0][e.index_polygon] == polygons[0][idx] ||
                polygons[0][e.index_polygon] == polygons[0][idx + 1])
                continue;

            auto pt = get_segments_intersection(polygons[0][idx], polygons[0][idx + 1], 
                polygons[0][e.index_polygon], polygons[1][e.index_hole]);
            if (pt != error_point)
            {
                is_valid = false;
                break;
            }
        }
        if (!is_valid)
        {
            edge_index ++;
            continue;
        }
        if (polygons[0][e.index_polygon] != polygons[0][0] && 
            polygons[0][e.index_polygon] != polygons[0][polygons[0].size() - 1])
        {
            auto pt = get_segments_intersection(polygons[0][0], polygons[0][polygons[0].size() - 1],
                polygons[0][e.index_polygon], polygons[1][e.index_hole]);
            if (pt != error_point)
            {
                edge_index ++;
                continue;
            }
        }

        for (std::size_t idx = 0; idx < polygons[1].size() - 1; ++ idx)
        {
            if (polygons[1][e.index_hole] == polygons[1][idx] ||
                polygons[1][e.index_hole] == polygons[1][idx + 1])
                continue;

            auto pt = get_segments_intersection(polygons[1][idx], polygons[1][idx + 1], 
                polygons[0][e.index_polygon], polygons[1][e.index_hole]);
            if (pt != error_point)
            {
                is_valid = false;
                break;
            }
        }
        if (!is_valid)
        {
            edge_index ++;
            continue;
        }
        if (polygons[1][e.index_hole] != polygons[1][0] && 
            polygons[1][e.index_hole] != polygons[1][polygons[1].size() - 1])
        {
            auto pt = get_segments_intersection(polygons[1][1], polygons[1][polygons[1].size() - 1],
                polygons[0][e.index_polygon], polygons[1][e.index_hole]);
            if (pt != error_point)
            {
                edge_index ++;
                continue;
            }
        }

        break;
    }

    bridge_edge e = bridges[edge_index];
    /*polygons[0][e.index_polygon].is_dup = true;
    polygons[1][e.index_hole].is_dup = true;*/
    std::vector<point> res;
    res.insert(res.end(), polygons[0].begin(), polygons[0].begin() + e.index_polygon + 1);
    res[res.size() - 1].is_dup = true;
    res.insert(res.end(), polygons[1].begin() + e.index_hole, polygons[1].end());
    res.insert(res.end(), polygons[1].begin(), polygons[1].begin() + e.index_hole + 1);
    res[res.size() - 1].is_dup = true;
    res.insert(res.end(), polygons[0].begin() + e.index_polygon, polygons[0].end());
    return res;
}

std::vector<float> get_triangulation(std::vector<std::vector<point>>& polygons)
{
    // the outer polygon must have vertices in counter clockwise order 
    // and the inner polygons must have vertices in clockwise order
    if (!is_counter_clockwise(polygons[0]))
        revert_order(polygons[0]);
    for (std::size_t idx = 1; idx < polygons.size(); ++idx)
        if (is_counter_clockwise(polygons[idx]))
            revert_order(polygons[idx]);
    
    auto degenerate_polygon = get_degenerate_polygon(polygons);
    triangles = get_triangulation(degenerate_polygon);
    std::vector<float> result;
    for (auto& tri: triangles)
    {
        result.push_back(tri->e1->a.x);
        result.push_back(tri->e1->a.y);
        result.push_back(tri->e1->b.x);
        result.push_back(tri->e1->b.y);
        result.push_back(tri->e2->b.x);
        result.push_back(tri->e2->b.y);
    }

    return result;
}

std::vector<triangle*> get_triangles(float mouse_x, float mouse_y, position& pos)
{
    std::vector<triangle*> result;
    for (auto& it : triangles)
    {
        if (point_in_triangle(it->e1->a, it->e1->b, it->e2->b, {mouse_x, mouse_y}))
        {
            result.push_back(it);
            pos = position::interior;
            break;
        }
        else if (point_on_triangle(it->e1->a, it->e1->b, it->e2->b, {mouse_x, mouse_y}))
        {
            result.push_back(it);
            pos = position::line;
        }
        else if (point_is_triangle_vertex(it->e1->a, it->e1->b, it->e2->b, {mouse_x, mouse_y}))
        {
            result.push_back(it);
            pos = position::point;
        }
    }
    return result;
}

void get_visibility(const point& q, const point& left, const point& right, const edge * e, std::vector<triangle*>& visibility)
{
    std::cout << "get visibility" << std::endl;
    // preprocessing of the edges, find through what edge we are looking into tri
    edge * e1 = nullptr;
    edge * e2 = nullptr;
    if (e == e->tri->e1)
    {
        e1 = e->tri->e2;
        e2 = e->tri->e3;
    }
    else if (e == e->tri->e2)
    {
        e1 = e->tri->e3;
        e2 = e->tri->e1;
    }
    else // e == tri->e3
    {
        e1 = e->tri->e1;
        e2 = e->tri->e2;
    }

    point p = e1->b;
    if (test_orientation(q, right, p) != orientation::left)
    {
        std::cout << "p is to the right of q - r" << std::endl;
        auto int_right = get_lines_intersection(q, right, e2->a, e2->b);
        if (e2->dual)
            get_visibility(q, left, int_right, e2->dual, visibility);
        else
        {
            std::cout << "e2 does not have dual" << std::endl;
            auto int_left = get_lines_intersection(q, left, e2->a, e2->b);
            visibility.push_back(get_triangle(q, int_left, int_right));
        }
    }
    else if (test_orientation(q, left, p) == orientation::right)
    {
        std::cout << "p is between q - r and q - l" << std::endl;
        if (e1->dual)
            get_visibility(q, e1->b, right, e1->dual, visibility);
        else
        {
            std::cout << "e1 does not have dual" << std::endl;
            auto intersection = get_lines_intersection(q, right, e1->a, e1->b);
            visibility.push_back(get_triangle(q, e1->b, intersection));
        }
        
        if (e2->dual)
            get_visibility(q, left, e2->a, e2->dual, visibility);
        else
        {
            std::cout << "e2 does not have dual" << std::endl;
            auto intersection = get_lines_intersection(q, left, e2->a, e2->b);
            visibility.push_back(get_triangle(q, intersection, e2->a));
        }
    }
    else // test_orientation(q, left, p) != orientation::right
    {
        std::cout << "p is to the left of q - l" << std::endl;
        auto int_left = get_lines_intersection(q, left, e1->a, e1->b);

        if (e1->dual)
            get_visibility(q, int_left, right, e1->dual, visibility);
        else
        {
            std::cout << "e1 does not have dual" << std::endl;
            auto int_right = get_lines_intersection(q, right, e1->a, e1->b);
            visibility.push_back(get_triangle(q, int_left, int_right));
        }
    }
}

void get_visibility_from_line(const std::vector<triangle*> tri, const point& q, std::vector<triangle*>& visibility)
{
    std::cout << "visi from line" << std::endl;
    for (auto& t : tri)
    {
        edge * e1 = nullptr;
        edge * e2 = nullptr;
        if (test_orientation(t->e1->a, t->e1->b, q) == orientation::collinear)
        {
            e1 = t->e2;
            e2 = t->e3;
        }
        else if (test_orientation(t->e2->a, t->e2->b, q) == orientation::collinear)
        {
            e1 = t->e3;
            e2 = t->e1;
        }
        else
        {
            e1 = t->e1;
            e2 = t->e2;
        }

        if (e1->dual)
            get_visibility(q, e1->b, e1->a, e1->dual, visibility);
        else 
            visibility.push_back(get_triangle(q, e1->a, e1->b));

        if (e2->dual)
            get_visibility(q, e2->b, e2->a, e2->dual, visibility);
        else 
            visibility.push_back(get_triangle(q, e2->a, e2->b));
    }
}

void get_visibility_from_vertex(const std::vector<triangle*> tri, const point& q, std::vector<triangle*>& visibility)
{
    for (auto& t : tri)
    {
        edge * e = nullptr;
        if (q == t->e1->a)
            e = t->e2;
        else if (q == t->e1->b)
            e = t->e3;
        else // q == t->e2->b
            e = t->e1;

        if (e->dual)
            get_visibility(q, e->b, e->a, e->dual, visibility);
        else 
            visibility.push_back(get_triangle(q, e->a, e->b));
    }
}

std::vector<triangle*> get_polygon_visibility_triangles(std::vector<std::vector<point>>& polygons, float px, float py)
{
    std::cout << "Get polygon visibility" << std::endl;
    if (triangles.size() == 0)
        get_triangulation(polygons);

    position pos;
    auto tri = get_triangles(px, py, pos);
    if (tri.size() == 0)
        return {};

    std::cout << "Valid triangle" << std::endl;

    point q {px, py};
    std::vector<triangle*> visibility;
    if (pos == position::interior)
    {
        if (tri[0]->e1->dual)
            get_visibility(q, tri[0]->e1->b, tri[0]->e1->a, tri[0]->e1->dual, visibility);
        else 
            visibility.push_back(get_triangle(q, tri[0]->e1->a, tri[0]->e1->b));

        if (tri[0]->e2->dual)
            get_visibility(q, tri[0]->e2->b, tri[0]->e2->a, tri[0]->e2->dual, visibility);
        else
            visibility.push_back(get_triangle(q, tri[0]->e2->a, tri[0]->e2->b));


        if (tri[0]->e3->dual)
            get_visibility(q, tri[0]->e3->b, tri[0]->e3->a, tri[0]->e3->dual, visibility);
        else
            visibility.push_back(get_triangle(q, tri[0]->e3->a, tri[0]->e3->b));
    }
    else if(pos == position::line)
        get_visibility_from_line(tri, q, visibility);
    else 
        get_visibility_from_vertex(tri, q, visibility);

    std::cout << "size :" << visibility.size() << std::endl;
    return visibility;
}

std::vector<float> get_polygon_visibility(std::vector<std::vector<point>>& points, float px, float py)
{
    auto triangles = get_polygon_visibility_triangles(points, px, py);
    std::vector<float> res;
    for (auto t : triangles)
    {
        res.push_back(t->e1->a.x);
        res.push_back(t->e1->a.y);
        res.push_back(t->e1->b.x);
        res.push_back(t->e1->b.y);
        res.push_back(t->e2->b.x);
        res.push_back(t->e2->b.y);
    }
    return res;
}
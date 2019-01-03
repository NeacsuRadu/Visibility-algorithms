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

    double min = 1000000.0;
    std::size_t idx_min = 0;
    point paux { 1000.0, origin.y };
    points.push_back(points[0]);
    for (std::size_t idx = 0; idx < points.size() - 1; ++idx)
    {
        point intersect = get_segments_intersection(points[idx], points[idx + 1], origin, paux);
        if (intersect == error_point)
            continue;

        double dist = distance(origin, intersect);
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

std::stack<stack_data> get_simple_polygon_visibility_stack(std::vector<point>& points, double px, double py)
{
    if (!point_in_polygon(points, {px, py}))
        return {};

    std::cout << "point in polygon" << std::endl;
    point q {px, py};
    simple_polygon_preprocessing(points, q);
   
    int idx = 1;
    int sz = points.size();
    std::stack<stack_data> st;
    st.push({points[0], true, 0});
    while (idx < sz)
    {
        if (test_orientation(q, points[index(idx - 1, sz)], points[index(idx, sz)]) != orientation::right)
        {
            std::cout << "vertex is to the left of the previous one, push" << std::endl;
            st.push({points[index(idx, sz)], true, idx});
            idx ++;
            continue;
        }

        std::cout << "vertex is to the right of the previous one" << std::endl;
        if (test_orientation(points[index(idx - 2, sz)], points[index(idx - 1, sz)], points[index(idx, sz)]) == orientation::right)
        {
            std::cout << "vertex is to the right of the -2 -1 line" << std::endl;
            point curr = points[index(idx - 1, sz)];
            idx ++;
            while (idx <= sz)
            {
                auto intersect = get_lines_intersection(q, curr, points[index(idx - 1, sz)], points[index(idx, sz)]);
                if (intersect != error_point && 
                    point_between_segment_vertices(intersect, points[index(idx - 1, sz)], points[index(idx, sz)]))
                {
                    st.push({intersect, false, idx - 1});
                    st.push({points[index(idx, sz)], true, idx});
                    break;
                }
                idx ++;
            }
            idx ++;
        }
        else 
        {
            std::cout << "vertex is to the left of -2 -1 line" << std::endl;
            point f1 = points[index(idx - 1, sz)]; // forward edge
            point f2 = points[index(idx, sz)];
            while (!st.empty())
            {
                auto data = st.top();
                auto intersection = get_segments_intersection(f1, f2, q, data.pt);
                if (intersection == error_point)
                {
                    auto ori = test_orientation(q, points[index(idx, sz)], points[index(idx + 1, sz)]);
                    if (ori == orientation::right)
                    {
                        std::cout << "idx + 1 is to the right of q idx " << std::endl;
                        f1 = f2;
                        f2 = points[index(idx + 1, sz)];
                        idx ++;
                        continue;
                    }
                    else 
                    {
                        if (test_orientation(f1, f2, points[index(idx + 1, sz)]) == orientation::right)
                        {
                            std::cout << "idx + 1 is to the right of idx - 1 idx " << std::endl;
                            auto m = get_lines_intersection(q, f2, data.pt, points[index(data.index + 1, sz)]);
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
                                std::cout << "while" << std::endl;
                                intersection = get_lines_intersection(q, f2, points[index(idx, sz)], points[index(idx + 1, sz)]);
                                if (intersection != error_point &&
                                    point_between_segment_vertices(intersection, points[index(idx, sz)], points[index(idx + 1, sz)]))
                                {
                                    f2 = points[index(idx + 1, sz)];
                                    f1 = points[index(idx, sz)];
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
                        std::cout << "stack popped" << std::endl;
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
                                point z = get_lines_intersection(intersection, st.top().pt, points[index(idx, sz)], points[index(idx - 1, sz)]);
                                if (z != error_point &&
                                    point_between_segment_vertices(z, points[index(idx, sz)], points[index(idx - 1, sz)]))
                                {
                                    st.push({z, false, idx - 1});
                                    st.push({points[index(idx, sz)], true, idx});
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

std::vector<point> get_simple_polygon_visibility(std::vector<point>& points, double px, double py)
{
    auto st = get_simple_polygon_visibility_stack(points, px, py);
    if (st.size() == 0)
        return {};

    point q {px, py};
    std::vector<point> result;
    point crr = st.top().pt;
    point last = crr;
    st.pop();
    while (!st.empty())
    {
        std::cout << "point: " << crr.x << " " << crr.y << std::endl;
        result.push_back(q);
        result.push_back(st.top().pt);
        result.push_back(crr);
        crr = st.top().pt;
        st.pop();
    }
    result.push_back(q);
    result.push_back(last);
    result.push_back(crr);
    return result;
}

struct bridge_edge 
{
    std::size_t index_polygon;
    std::size_t index_hole;
    double distance = 0.0;
};

std::vector<triangle*> triangles;
bool bridge_intersects_polygon(const std::vector<point> polygon, const point& pp, const point& ph)
{
    auto sz = polygon.size();
    if (pp != polygon[0] && pp != polygon[sz - 1])
    {
        auto pt = get_segments_intersection(pp, ph, polygon[0], polygon[sz - 1]);
        if (pt != error_point)
            return true;
    }

    for (std::size_t idx = 0; idx < sz - 1; ++ idx)
    {
        if (pp == polygon[idx] || pp == polygon[idx + 1])
            continue;

        auto pt = get_segments_intersection(pp, ph, polygon[idx], polygon[idx + 1]);
        if (pt != error_point)
            return true;
    }

    return false;
}

bool bridge_intersects_holes(const point& pp, const point&ph, 
    const std::vector<std::vector<point>> holes)
{
    for (std::size_t idx = 1; idx < holes.size(); ++idx)
        if (bridge_intersects_polygon(holes[idx], pp, ph))
            return true;
    return false;
}

std::vector<point> get_degenerate_polygon(const std::vector<std::vector<point>>& in)
{   
    std::cout << "get degenerate polygon - in" << std::endl;
    //std::cout << "in" << std::endl;
    if (in.size() == 1)
        return in[0];

    auto polygons = in;
    auto dp = polygons[0]; // degenerate polygon
    for(std::size_t _ = 1; _ < in.size(); ++_)
    {
        std::cout << "iteration: " << _ << std::endl;
        for (std::size_t hole_nr = 1; hole_nr < polygons.size(); ++ hole_nr)
        {
            //std::cout << "nr: " << hole_nr << std::endl;
            auto& hole = polygons[hole_nr];
            for (auto pt: hole)
                std::cout << pt.x << " " << pt.y << " / ";
            std::cout << std::endl;

            std::vector<bridge_edge> bridges;
            for (std::size_t idx_p = 0; idx_p < dp.size(); ++ idx_p)
                for (std::size_t idx_h = 0; idx_h < hole.size(); ++ idx_h)
                    bridges.push_back({idx_p, idx_h, distance(dp[idx_p], hole[idx_h])});

            std::sort(bridges.begin(), bridges.end(), 
                [](const bridge_edge& e1, const bridge_edge& e2) {
                return e1.distance < e2.distance; });

           
            std::size_t idx_edge = 0;
            std::cout << " algorithm start" << std::endl;
            for (const auto& e: bridges)
            {
                std::cout << dp[e.index_polygon].x << " " << dp[e.index_polygon].y << " / "  << hole[e.index_hole].x <<  " " << hole[e.index_hole].y << " / " << e.distance << std::endl;

                if (bridge_intersects_polygon(dp, dp[e.index_polygon], hole[e.index_hole]) ||
                    //bridge_intersects_polygon(hole, hole[e.index_hole], dp[e.index_polygon]) ||
                    bridge_intersects_holes(hole[e.index_hole], dp[e.index_polygon], polygons))
                    idx_edge ++;
                else
                {
                    std::cout << "Fould good one" << std::endl;
                    break;
                }
            }

            std::cout << idx_edge << std::endl;
            if (idx_edge == bridges.size())
                continue;

            auto& e = bridges[idx_edge];
            std::vector<point> aux;
            aux.insert(aux.end(), dp.begin(), dp.begin() + e.index_polygon + 1);
            aux[aux.size() - 1].is_dup = true;
            aux.insert(aux.end(), hole.begin() + e.index_hole, hole.end());
            aux.insert(aux.end(), hole.begin(), hole.begin() + e.index_hole + 1);
            aux[aux.size() - 1].is_dup = true;
            aux.insert(aux.end(), dp.begin() + e.index_polygon, dp.end());

            dp.swap(aux);

            if (hole_nr < polygons.size() - 1)
                polygons[hole_nr].swap(polygons[polygons.size() - 1]);
            polygons.pop_back();
            break;
        }
    }

    std::cout << "get degenerate polygon - out " << std::endl;
    return dp;
}

std::vector<point> get_triangulation(std::vector<std::vector<point>>& polygons)
{
    // the outer polygon must have vertices in counter clockwise order 
    // and the inner polygons must have vertices in clockwise order
    if (!is_counter_clockwise(polygons[0]))
        revert_order(polygons[0]);
    for (std::size_t idx = 1; idx < polygons.size(); ++idx)
        if (is_counter_clockwise(polygons[idx]))
            revert_order(polygons[idx]);
    
    for (std::size_t i = 0; i < polygons.size(); ++i)
    {
        for (std::size_t j = 0; j < polygons[i].size(); ++j)
            std::cout << polygons[i][j].x << " " << polygons[i][j].y << " / " ;
        std::cout << std::endl;
    }

    auto degenerate_polygon = get_degenerate_polygon(polygons);
    std::cout << std::endl << "Degenerate polygon" << std::endl;
    for (auto pt: degenerate_polygon)
        std::cout << pt.x << " " << pt.y << " / " << std::endl;
    triangles = get_triangulation(degenerate_polygon);
    std::vector<point> result;
    //std::cout << "TRIANGULATION: " << std::endl;
    for (auto& tri: triangles)
    {
        //std::cout << "tri: " << tri->e1->a.x << " " << tri->e1->a.y << " -- " << tri->e1->b.x << " " << tri->e1->b.y << " -- " << tri->e2->b.x << " " << tri->e2->b.y << std::endl;
        result.push_back(tri->e1->a);
        result.push_back(tri->e1->b);
        result.push_back(tri->e2->b);
    }

    return result;
}

std::vector<triangle*> get_triangles(double mouse_x, double mouse_y, position& pos)
{
    //std::cout << "Get triangles, viewpoint: " << mouse_x << " " << mouse_y << std::endl;
    std::vector<triangle*> result;
    for (auto& it : triangles)
    {
        if (point_is_in_tri_interior(it->e1->a, it->e1->b, it->e2->b, {mouse_x, mouse_y}))
        {
           // std::cout << "interior point" << std::endl;
            result.push_back(it);
            pos = position::interior;
            break;
        }
        else if (point_on_triangle(it->e1->a, it->e1->b, it->e2->b, {mouse_x, mouse_y}))
        {
            //std::cout << "point on line" << std::endl;
            result.push_back(it);
            pos = position::line;
        }
        else if (point_is_triangle_vertex(it->e1->a, it->e1->b, it->e2->b, {mouse_x, mouse_y}))
        {
            //std::cout << "point is vertex" << std::endl;
            result.push_back(it);
            pos = position::point;
        }
    }
    return result;
}

void get_visibility(const point& q, const point& left, const point& right, const edge * e, std::vector<triangle*>& visibility)
{
    //std::cout << "get visibility" << std::endl;
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
       // std::cout << "p is to the right of q - r" << std::endl;
        auto int_right = get_lines_intersection(q, right, e2->a, e2->b);
        if (e2->dual)
            get_visibility(q, left, int_right, e2->dual, visibility);
        else
        {
            //std::cout << "e2 does not have dual" << std::endl;
            auto int_left = get_lines_intersection(q, left, e2->a, e2->b);
            visibility.push_back(get_triangle(q, int_right, int_left));
        }
    }
    else if (test_orientation(q, left, p) == orientation::right)
    {
        //std::cout << "p is between q - r and q - l" << std::endl;
        if (e1->dual)
            get_visibility(q, e1->b, right, e1->dual, visibility);
        else
        {
            //std::cout << "e1 does not have dual" << std::endl;
            auto intersection = get_lines_intersection(q, right, e1->a, e1->b);
            visibility.push_back(get_triangle(q, intersection, e1->b));
        }
        
        if (e2->dual)
            get_visibility(q, left, e2->a, e2->dual, visibility);
        else
        {
            //std::cout << "e2 does not have dual" << std::endl;
            auto intersection = get_lines_intersection(q, left, e2->a, e2->b);
            visibility.push_back(get_triangle(q, e2->a, intersection));
        }
    }
    else // test_orientation(q, left, p) != orientation::right
    {
        //std::cout << "p is to the left of q - l" << std::endl;
        auto int_left = get_lines_intersection(q, left, e1->a, e1->b);

        if (e1->dual)
            get_visibility(q, int_left, right, e1->dual, visibility);
        else
        {
            //std::cout << "e1 does not have dual" << std::endl;
            auto int_right = get_lines_intersection(q, right, e1->a, e1->b);
            visibility.push_back(get_triangle(q, int_right, int_left));
        }
    }
}

void get_visibility_from_line(const std::vector<triangle*> tri, const point& q, std::vector<triangle*>& visibility)
{
    //std::cout << "visi from line" << std::endl;
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
    //std::cout << "get visibility from vertex: " << tri.size() << std::endl;
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

std::vector<triangle*> get_polygon_visibility_triangles(std::vector<std::vector<point>>& polygons, double px, double py)
{
    //std::cout << "Get polygon visibility triangles" << std::endl;
    if (triangles.size() == 0)
        get_triangulation(polygons);

    position pos;
    auto tri = get_triangles(px, py, pos);
    if (tri.size() == 0)
        return {};

    //std::cout << "Found " << tri.size() << " Valid triangles" << std::endl;

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

    //std::cout << "size :" << visibility.size() << std::endl;
    return visibility;
}

std::vector<point> get_polygon_visibility(std::vector<std::vector<point>>& points, double px, double py)
{
    //std::cout << "Get polygon visibility" << std::endl;
    auto triangles = get_polygon_visibility_triangles(points, px, py);
    //std::cout << "Visibility has " << triangles.size() <<  " triangles" << std::endl;
    std::vector<point> result;
    for (auto t : triangles)
    {
        result.push_back(t->e1->a);
        result.push_back(t->e1->b);
        result.push_back(t->e2->b);
    }
    return result;
}

bool is_point_visible(const std::vector<triangle*>& triangles, const point& p)
{
    for (auto& t: triangles)
    {
        //std::cout << "tri: " << t->e1->a.x << " " << t->e1->a.y << " -- " << t->e1->b.x << " " << t->e1->b.y << " -- " << t->e2->b.x << " " << t->e2->b.y << std::endl;
        if (point_is_triangle_vertex(t->e1->a, t->e1->b, t->e2->b, p) || 
            point_in_triangle(p, t))
            return true;
        //std::cout << "not inside" << std::endl;
    }

    return false;
}

std::vector<point> get_visible_points(const std::vector<std::vector<point>>& points,
    const std::vector<triangle*>& triangles, const point& p)
{
    /*std::cout << "get visibile points" << std::endl;
    std::cout << "no of tri: " << triangles.size() << std::endl;
    for (auto t: triangles)
        std::cout << "tri: " << t->e1->a.x << " " << t->e1->a.y << " -- " << t->e1->b.x << " " << t->e1->b.y << " -- " << t->e2->b.x << " " << t->e2->b.y << std::endl;*/
    std::vector<point> res;
    auto sz = points.size();
    for (std::size_t i = 0; i < sz; ++ i)
    {
        auto sz_poly = points[i].size();
        for (std::size_t j = 0; j < sz_poly; ++j)
        {
            //std::cout << "point: " << points[i][j].x << " " << points[i][j].y << std::endl;
            if (points[i][j] == p) // nu ne intereseaza punctul din care ne uitam
                continue;

            if (is_point_visible(triangles, points[i][j]))
                res.push_back(points[i][j]);
        }
    }
    return res;
}

void get_visible_vertices(const point& q, const point& left, const point& right, const edge * e, std::vector<point>& vertices)
{
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
        if (e2->dual)
            get_visible_vertices(q, left, right, e2->dual, vertices);
    }
    else if(test_orientation(q, left, p) == orientation::right)
    {
        if (e1->dual)
            get_visible_vertices(q, p, right, e1->dual, vertices);
        vertices.push_back(p);
        if (e2->dual)
            get_visible_vertices(q, left, p, e2->dual, vertices);
    }
    else
    {
        if (e1->dual)
            get_visible_vertices(q, left, right, e1->dual, vertices);
    }
}

std::vector<point> get_visible_vertices(const std::vector<triangle *>& tri, const point& p)
{
    std::vector<point> vertices;
    for (auto& t: tri)
    {
        edge * e = nullptr;
        if (p == t->e1->a)
            e = t->e2;
        else if (p == t->e2->a)
            e = t->e3;
        else
            e = t->e1;

        vertices.push_back(e->a);
        if (e->dual)
            get_visible_vertices(p, e->b, e->a, e->dual, vertices);
        vertices.push_back(e->b);
    }
    return vertices;
}

graph<point, compare_pt> * get_visibility_graph(std::vector<std::vector<point>>& points)
{   
    std::size_t sz = points.size();
    std::vector<point> nodes;
    for (std::size_t i = 0; i < sz; ++i)
        nodes.insert(nodes.end(), points[i].begin(), points[i].end());
    //std::cout << "no nodes: " << nodes.size() << std::endl;
    std::cout << "no nodes: " << nodes.size() << std::endl;
    graph<point, compare_pt> * g = new graph<point, compare_pt>(nodes);

    for (std::size_t i = 0; i < sz; ++i)
    {
        std::size_t sz_poly = points[i].size();
        for (std::size_t j = 0; j < sz_poly; ++j)
        {
            position pos;
            auto tri = get_triangles(points[i][j].x, points[i][j].y, pos);
            if (pos != position::point)
                throw std::runtime_error("vs graph, point no vertex");

           // std::cout << "point is vertex for " << tri.size() << std::endl;
            auto vertices = get_visible_vertices(tri, points[i][j]);
            for (auto& vert: vertices)
                g->add_edge(points[i][j], vert, distance(points[i][j], vert));
        }
    }

    std::cout << "Before compute all paths" << std::endl;
    g->compute_all_paths();
    return g;
}

bool ceva ( triangle * t)
{
    return test_orientation(t->e1->a, t->e1->b, t->e2->b) == orientation::left && 
        test_orientation(t->e2->a, t->e2->b, t->e3->b) == orientation::left &&
        test_orientation(t->e3->a, t->e3->b, t->e1->b) == orientation::left;
}

std::vector<point> get_path_ab(const point& a, const point& b, std::vector<std::vector<point>>& polys, graph<point, compare_pt> * g)
{
    auto visit_a = get_polygon_visibility_triangles(polys, a.x, a.y);
    if (is_point_visible(visit_a, b))
        return { a, b };

    //std::cout << "Points are not visible" << std::endl;
    auto visit_b = get_polygon_visibility_triangles(polys, b.x, b.y);

    auto visible_points_a = get_visible_points(polys, visit_a, a);
    auto visible_points_b = get_visible_points(polys, visit_b, b);

    auto dist = 10000000000.0; // big enough
    point p1, p2;
    for (auto& pt_a: visible_points_a)
    {
        auto distance_pt_a = distance(a, pt_a);
        for (auto& pt_b: visible_points_b)
        {
           auto distance_pt_b = distance(b, pt_b);
           double distance_graph = 0.0;
           g->get_path(pt_a, pt_b, distance_graph);
           if (dist > distance_pt_a + distance_pt_b + distance_graph)
           {
               dist = distance_pt_a + distance_pt_b + distance_graph;
               p1 = pt_a;
               p2 = pt_b;
           }
        }
    }

    std::vector<point> path = {a};
    auto path_graph = g->get_path(p1, p2, dist);
    //std::cout << "path_graph has: " << path_graph.size() << " nodes" << std::endl;
    path.insert(path.end(), path_graph.begin(), path_graph.end());
    path.push_back(b);

    //std::cout << "path has " << path.size() << "points" << std::endl;
    return path;
}

std::vector<point> get_degenerate_polygon_b(const std::vector<std::vector<point>>& points)
{
    auto vec = points;
    if (!is_counter_clockwise(vec[0]))
        revert_order(vec[0]);
    for (std::size_t idx = 1; idx < vec.size(); ++idx)
        if (is_counter_clockwise(vec[idx]))
            revert_order(vec[idx]);

    std::vector<point> res;
    auto a = get_degenerate_polygon(vec);
    for (std::size_t idx = 0; idx < a.size(); ++idx)
    {
        if (idx == 0 )
            res.push_back(a[idx]);
        else
        {
            res.push_back(a[idx]);
            res.push_back(a[idx]);
        }
    }
    res.push_back(a[0]);
    return res;
}
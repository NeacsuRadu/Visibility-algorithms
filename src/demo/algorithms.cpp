#include "algorithms.h"
#include "geometry.h"
#include "triangulation.h"
#include "polygon_helpers.h"
#include "triangulated_polygon_visibility.h"

#include <stack>
#include <iostream>
#include <algorithm>

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
            auto vertices = triangulated_polygon_visibility::get_instance().get_visibile_vertices_from_vertex(points[i][j]);
            for (auto& vert: vertices)
                g->add_edge(points[i][j], vert, distance(points[i][j], vert));
        }
    }

    std::cout << "Before compute all paths" << std::endl;
    g->compute_all_paths();
    return g;
}

std::vector<point> get_path_ab(const point& a, const point& b, std::vector<std::vector<point>>& polys, graph<point, compare_pt> * g)
{
    auto visit_a = triangulated_polygon_visibility::get_instance().get_visibility(a);
    if (is_point_visible(visit_a, b))
        return { a, b };

    //std::cout << "Points are not visible" << std::endl;
    auto visit_b = triangulated_polygon_visibility::get_instance().get_visibility(b);

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
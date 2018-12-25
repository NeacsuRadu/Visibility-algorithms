#pragma once
#include "geometry.h"
#include "graph.h"

#include <vector>


//std::vector<float> get_simple_polygon_visibility(const std::vector<float>& points, float px, float py);
std::vector<point> get_triangulation(std::vector<std::vector<point>>& points);
std::vector<triangle*> get_triangles(long long mouse_x, long long mouse_y, position& pos);
std::vector<point> get_polygon_visibility(std::vector<std::vector<point>>& points, long long px, long long py);
std::vector<triangle*> get_polygon_visibility_triangles(std::vector<std::vector<point>>& points, long long px, long long py);
graph<point, compare_pt> * get_visibility_graph(std::vector<std::vector<point>>& points);
std::vector<point> get_path_ab(const point& a, const point& b, std::vector<std::vector<point>>& polys, graph<point, compare_pt> * g);
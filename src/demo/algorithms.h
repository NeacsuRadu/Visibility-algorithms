#pragma once
#include "geometry.h"
#include "graph.h"

#include <vector>


std::vector<point> get_simple_polygon_visibility(std::vector<point>& points, double px, double py);
std::vector<point> get_triangulation(std::vector<std::vector<point>>& points);
std::vector<triangle*> get_triangles(double mouse_x, double mouse_y, position& pos);
std::vector<point> get_polygon_visibility(std::vector<std::vector<point>>& points, double px, double py);
std::vector<triangle*> get_polygon_visibility_triangles(std::vector<std::vector<point>>& points, double px, double py);
graph<point, compare_pt> * get_visibility_graph(std::vector<std::vector<point>>& points);
std::vector<point> get_path_ab(const point& a, const point& b, std::vector<std::vector<point>>& polys, graph<point, compare_pt> * g);
std::vector<point> get_degenerate_polygon_b(const std::vector<std::vector<point>>& points);
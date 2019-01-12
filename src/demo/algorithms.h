#pragma once
#include "geometry.h"
#include "graph.h"

#include <vector>

graph<point, compare_pt> * get_visibility_graph(std::vector<std::vector<point>>& points);
std::vector<point> get_path_ab(const point& a, const point& b, std::vector<std::vector<point>>& polys, graph<point, compare_pt> * g);
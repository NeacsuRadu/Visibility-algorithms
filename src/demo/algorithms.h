#pragma once
#include "geometry.h"

#include <vector>


std::vector<float> get_simple_polygon_visibility(const std::vector<float>& points, float px, float py);
std::vector<float> get_triangulation(std::vector<std::vector<point>>& points);
std::vector<triangle*> get_triangles(float mouse_x, float mouse_y, position& pos);
std::vector<float> get_polygon_visibility(std::vector<std::vector<point>>& points, float px, float py);
std::vector<triangle*> get_polygon_visibility_triangles(std::vector<std::vector<point>>& points, float px, float py);
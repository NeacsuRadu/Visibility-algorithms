#pragma once
#include "geometry.h"

#include <vector>


std::vector<float> get_simple_polygon_visibility(const std::vector<float>& points, float px, float py);
std::vector<float> get_triangulation(const std::vector<float>& points);
std::vector<triangle*> get_triangles(float mouse_x, float mouse_y, position& pos);
std::vector<float> get_polygon_visibility(const std::vector<float>& points, float px, float py);
std::vector<triangle*> get_polygon_visibility_triangles(const std::vector<float>& points, float px, float py);
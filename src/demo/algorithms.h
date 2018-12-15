#pragma once
#include "geometry.h"

#include <vector>


std::vector<float> get_simple_polygon_visibility(const std::vector<float>& points, float px, float py);
std::vector<float> get_triangulation(const std::vector<float>& points);
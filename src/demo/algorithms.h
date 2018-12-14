#pragma once
#include <vector>
#include "geometry.h"

bool is_counter_clockwise(const std::vector<point>& points);
void to_counter_clockwise(std::vector<point>& points);
void simple_polygon_preprocessing(std::vector<point>& points, const point& origin);
std::vector<float> get_simple_polygon_visibility(const std::vector<float>& points, float px, float py);

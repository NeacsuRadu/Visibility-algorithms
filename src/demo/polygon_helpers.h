#pragma once
#include "geometry.h"

#include <vector>

bool is_counter_clockwise(const std::vector<point>& polygon);
void revert_order(std::vector<point>& points);

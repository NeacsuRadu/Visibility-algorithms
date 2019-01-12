#pragma once
#include "geometry.h"

#include <vector>

class visibility_algorithm
{
public:
    virtual ~visibility_algorithm() = default;
    virtual void preprocess_polygons(const std::vector<std::vector<point>>& polygons) = 0;
    virtual std::vector<triangle*> get_visibility(const point& view) = 0;
    virtual std::vector<point> get_visibile_vertices_from_vertex(const point& view) = 0;
};
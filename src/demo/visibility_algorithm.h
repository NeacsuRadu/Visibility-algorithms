#pragma once
#include "geometry.h"

#include <vector>

struct display_data
{
    std::vector<triangle*> m_triangles;
    std::vector<point>     m_points;
};

class visibility_algorithm
{
public:
    virtual ~visibility_algorithm() = default;
    virtual void preprocess_polygons(const std::vector<std::vector<point>>& polygons) = 0;
    virtual std::vector<triangle*> get_visibility(const point& view) = 0;
    virtual std::vector<point> get_visibile_vertices_from_vertex(const point& view) = 0;
    virtual std::vector<display_data> get_display_data_steps() = 0;
};
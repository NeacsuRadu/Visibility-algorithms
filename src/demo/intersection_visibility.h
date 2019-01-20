#pragma once
#include "singleton.h"
#include "visibility_algorithm.h"

class intersect_visibility :
    public singleton<intersect_visibility>,
    public visibility_algorithm
{
public:
    void preprocess_polygons(const std::vector<std::vector<point>>& polygons) override { m_polygons = polygons; };
    std::vector<triangle*> get_visibility(const point& view) override;
    std::vector<point> get_visibile_vertices_from_vertex(const point& view) override { return {}; };
private:
    point _get_visible_point(const point& view, const point& pt);

    std::vector<std::vector<point>> m_polygons;
};
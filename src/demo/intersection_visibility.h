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
    std::vector<display_data> get_display_data_steps() override { return m_display_data_steps; };
private:
    point _get_visible_point(const std::vector<point>& visi, const point& view, const point& pt, bool save = false);
    void _save_data(const std::vector<triangle*>& triangles, const std::vector<point>& points);
    void _save_data(const point& view, const std::vector<point>& triangles, const std::vector<point>& points);

    std::vector<std::vector<point>> m_polygons;
    std::vector<display_data> m_display_data_steps;
};
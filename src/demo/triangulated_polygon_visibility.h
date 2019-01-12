#pragma once
#include "singleton.h"
#include "visibility_algorithm.h"

class triangulated_polygon_visibility:
    public singleton<triangulated_polygon_visibility>,
    public visibility_algorithm
{
public:
    void preprocess_polygons(const std::vector<std::vector<point>>& polygons) override {};
    std::vector<triangle*> get_visibility(const point& view) override;
    std::vector<point> get_visibile_vertices_from_vertex(const point& view) override;
private:
    std::vector<triangle*> _get_triangles_containing_point(const point& pt, position& pos);
    void _get_visibility_from_line(const std::vector<triangle*>& tris, const point& view, std::vector<triangle*>& visi);
    void _get_visibility_from_vertex(const std::vector<triangle*>& tris, const point& view, std::vector<triangle*>& visi);
    void _get_visibility_through_edge(const point& view, const point& left, const point& right, const edge * e, std::vector<triangle*>& vs);
    void _get_visibility_through_edge(const point& view, const point& left, const point& right, const edge * e, std::vector<point>& vertices);
};
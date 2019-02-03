#pragma once
#include "singleton.h"
#include "visibility_algorithm.h"

#include <stack>

class simple_polygon_visibility:
    public singleton<simple_polygon_visibility>,
    public visibility_algorithm
{
    struct stack_data 
    {
        point pt;
        bool is_vertex = false;
        int index = -1;
    };
public:
    void preprocess_polygons(const std::vector<std::vector<point>>& polygons) override;
    std::vector<triangle*> get_visibility(const point& view) override;
    std::vector<point> get_visibile_vertices_from_vertex(const point& view) override { return {}; }
    std::vector<display_data> get_display_data_steps() override { return m_display_data_steps; };
private:
    void _shift_polygon_points(const point& view);
    void _save_stack(const std::vector<point>& points, const point& view, std::stack<stack_data> st, bool last = false);
    std::stack<stack_data> _get_visibility_stack(const point& view);

    std::vector<point> m_polygon;

    std::vector<display_data> m_display_data_steps;
};
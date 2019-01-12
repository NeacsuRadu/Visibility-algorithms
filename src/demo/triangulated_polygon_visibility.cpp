#include "triangulated_polygon_visibility.h"
#include "triangulation.h"

triangulated_polygon_visibility * singleton<triangulated_polygon_visibility>::instance = nullptr;

std::vector<triangle*> triangulated_polygon_visibility::get_visibility(const point& view)
{
    position pos = position::interior;
    auto tri = _get_triangles_containing_point(view, pos);
    if (tri.size() == 0)
        return {};

    std::vector<triangle*> visibility;
    if (pos == position::interior)
    {
        if (tri.size() > 1)
            throw std::runtime_error("interior point contained by more than one triangle");

        if (tri[0]->e1->dual)
            _get_visibility_through_edge(view, tri[0]->e1->b, tri[0]->e1->a, tri[0]->e1->dual, visibility);
        else 
            visibility.push_back(get_triangle(view, tri[0]->e1->a, tri[0]->e1->b));

        if (tri[0]->e2->dual)
            _get_visibility_through_edge(view, tri[0]->e2->b, tri[0]->e2->a, tri[0]->e2->dual, visibility);
        else
            visibility.push_back(get_triangle(view, tri[0]->e2->a, tri[0]->e2->b));

        if (tri[0]->e3->dual)
            _get_visibility_through_edge(view, tri[0]->e3->b, tri[0]->e3->a, tri[0]->e3->dual, visibility);
        else
            visibility.push_back(get_triangle(view, tri[0]->e3->a, tri[0]->e3->b));
    }
    else if (pos == position::line)
        _get_visibility_from_line(tri, view, visibility);
    else // pos == position::point
        _get_visibility_from_vertex(tri, view, visibility);

    return visibility;
}

std::vector<point> triangulated_polygon_visibility::get_visibile_vertices_from_vertex(const point& view)
{
    position pos;
    auto tris = _get_triangles_containing_point(view, pos);
    if (pos != position::point)
        throw std::runtime_error("point not vertex");

    std::vector<point> vertices;
    for (const auto& t: tris)
    {
        edge * e = nullptr;
        if (view == t->e1->a)
            e = t->e2;
        else if (view == t->e2->a)
            e = t->e3;
        else 
            e = t->e1;

        vertices.push_back(e->a);
        if (e->dual)
            _get_visibility_through_edge(view, e->b, e->a, e->dual, vertices);
        vertices.push_back(e->b);
    }

    return vertices;
}

void triangulated_polygon_visibility::_get_visibility_through_edge(const point& view, const point& left, const point& right, const edge * e, std::vector<point>& vertices)
{
    edge * e1 = nullptr;
    edge * e2 = nullptr;
    triangle * t = e->tri;
    if (e == t->e1)
    {
        e1 = t->e2;
        e2 = t->e3;
    }
    else if (e == t->e2)
    {
        e1 = t->e3;
        e2 = t->e1;
    }
    else // e == t->e3
    {
        e1 = t->e1;
        e2 = t->e2;
    }

    point p = e1->b;
    if (test_orientation(view, right, p) != orientation::left)
    {
        if (e2->dual)
            _get_visibility_through_edge(view, left, right, e2->dual, vertices);
    }
    else if (test_orientation(view, left, p) == orientation::right)
    {
        if (e1->dual)
            _get_visibility_through_edge(view, p, right, e1->dual, vertices);
        vertices.push_back(p);
        if (e2->dual)
            _get_visibility_through_edge(view, left, p, e2->dual, vertices);
    }
    else // orientation view, left, p != orientation::right
    {
        if (e1->dual)
            _get_visibility_through_edge(view, left, right, e1->dual, vertices);
    }
}

void triangulated_polygon_visibility::_get_visibility_from_line(const std::vector<triangle*>& tris, const point& view, std::vector<triangle*>& visi)
{
    for (const auto& t: tris)
    {
        // find the edge that contains view and set e1 and e2 with the other ones
        edge * e1 = nullptr;
        edge * e2 = nullptr;
        if (test_orientation(t->e1->a, t->e1->b, view) == orientation::collinear)
        {
            e1 = t->e2;
            e2 = t->e3;
        }
        else if (test_orientation(t->e2->a, t->e2->b, view) == orientation::collinear)
        {
            e1 = t->e3;
            e2 = t->e1;
        }
        else 
        {
            e1 = t->e1;
            e2 = t->e2;
        }

        if (e1->dual)
            _get_visibility_through_edge(view, e1->b, e1->a, e1->dual, visi);
        else 
            visi.push_back(get_triangle(view, e1->a ,e1->b));

        if (e2->dual)
            _get_visibility_through_edge(view, e2->b, e2->a, e2->dual, visi);
        else
            visi.push_back(get_triangle(view, e2->a, e2->b));
    }
}

void triangulated_polygon_visibility::_get_visibility_from_vertex(const std::vector<triangle*>& tris, const point& view, std::vector<triangle*>& visibility)
{
    for (const auto& t: tris)
    {
        edge * e = nullptr;
        if (view == t->e1->a)
            e = t->e2;
        else if (view == t->e2->a)
            e = t->e3;
        else 
            e = t->e1;

        if (e->dual)
            _get_visibility_through_edge(view, e->b, e->a, e->dual, visibility);
        else 
            visibility.push_back(get_triangle(view, e->a, e->b));
    }
}

void triangulated_polygon_visibility::_get_visibility_through_edge(const point& view, const point& left, const point& right, const edge * e, std::vector<triangle*>& vs)
{
    // preprocessing of the edges, find through what edge we are looking into the triangle
    edge * e1 = nullptr;
    edge * e2 = nullptr;
    triangle * t = e->tri;
    if (e == t->e1)
    {
        e1 = t->e2;
        e2 = t->e3;
    }
    else if (e == t->e2)
    {
        e1 = t->e3;
        e2 = t->e1;
    }
    else // e == t->e3
    {
        e1 = t->e1;
        e2 = t->e2;
    }

    point p = e1->b;
    if (test_orientation(view, right, p) != orientation::left)
    {
        auto int_right = get_lines_intersection(view, right, e2->a, e2->b);
        if (e2->dual)
            _get_visibility_through_edge(view, left, int_right, e2->dual, vs);
        else
        {
            auto int_left = get_lines_intersection(view, left, e2->a, e2->b);
            vs.push_back(get_triangle(view, int_right, int_left));
        }
    }
    else if (test_orientation(view, left, p) == orientation::right)
    {
        if (e1->dual)
            _get_visibility_through_edge(view, e1->b, right, e1->dual, vs);
        else
        {
            auto inter = get_lines_intersection(view, right, e1->a, e1->b);
            vs.push_back(get_triangle(view, inter, e1->b));
        }

        if (e2->dual)
            _get_visibility_through_edge(view, left, e2->a, e2->dual, vs);
        else
        {
            auto inter = get_lines_intersection(view, left, e2->a, e2->b);
            vs.push_back(get_triangle(view, e2->a, inter));
        }
    }
    else // view, left, p != orientation::right
    {
        auto int_left = get_lines_intersection(view, left, e1->a, e1->b);
        if (e1->dual)
            _get_visibility_through_edge(view, int_left, right, e1->dual, vs);
        else
        {
            auto int_right = get_lines_intersection(view, right, e1->a, e1->b);
            vs.push_back(get_triangle(view, int_right, int_left));

        }
    }
}


std::vector<triangle*> triangulated_polygon_visibility::_get_triangles_containing_point(const point& pt, position& pos)
{
    auto triangles = triangulation::get_instance().get_triangulation();
    decltype(triangles) res;
    for (auto& tri: triangles)
    {
        if (point_is_triangle_vertex(tri->e1->a, tri->e2->a, tri->e3->a, pt))
        {
            res.push_back(tri);
            pos = position::point;
        }
        else if (point_on_triangle(tri->e1->a, tri->e2->a, tri->e3->a, pt))
        {
            res.push_back(tri);
            pos = position::line;
        }
        else if (point_is_in_tri_interior(tri->e1->a, tri->e2->a, tri->e3->a, pt))
        {
            res.push_back(tri);
            pos = position::interior;
        }
    }

    return res;
}
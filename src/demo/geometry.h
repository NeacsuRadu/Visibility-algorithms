#pragma once
#include <cmath>
#include <iostream>
enum class orientation: int
{
    right = -1,
    collinear,
    left
};

struct point
{
    double x = 0.0;
    double y = 0.0;
};

static point error_point = {-9999.999999, -9999.99999};
inline bool operator == (const point& p, const point& q)
{
    return p.x == q.x && p.y == q.y;
}

inline bool operator != (const point& p, const point& q)
{
    return !(p == q);
}

struct vertex 
{
    point coord;

};

struct line_equation
{
    /* ax + by + c = 0 */
    double a = 0.0;
    double b = 0.0;
    double c = 0.0;
};

inline double determinant(
    double l11, double l12, 
    double l21, double l22)
{
    return l11*l22 - l12*l21;
}

inline double determinant(
    double l11, double l12, double l13,
    double l21, double l22, double l23,
    double l31, double l32, double l33)
{
    return l11*l22*l33 + l12*l23*l31 + l21*l32*l13 
        -  l13*l22*l31 - l11*l32*l23 - l33*l21*l12;
}

inline double distance(const point& p, const point& q)
{
    return std::sqrt(std::pow(p.x - q.x, 2) + std::pow(p.y - q.y, 2));
}

inline orientation test_orientation(const point& p, const point& q, const point& r)
{
    auto det = determinant(1, 1, 1, p.x, q.x, r.x, p.y, q.y, r.y);
    if (det < 0)
        return orientation::right;
    if (det == 0)
        return orientation::collinear;
    return orientation::left;
}

inline bool point_between_segment_vertices(const point& p, const point& s1, const point& s2)
{
    return (s1.x <= p.x && p.x <= s2.x || s2.x <= p.x && p.x <= s1.x)
        && (s1.y <= p.y && p.y <= s2.y || s2.y <= p.y && p.y <= s1.y);
}

inline line_equation get_line_equation(const point& p, const point& q)
{
    if (p == q)
        throw std::runtime_error("line equation called with two equal point");

    if (p.x == q.x)
        return {1.0, 0.0, -p.x};
    if (p.y == q.y)
        return {0.0, 1.0, -p.y};
    return {p.y - q.y, q.x - p.x, p.x * q.y - q.x * p.y};
}

inline point get_lines_intersection(const line_equation& l1, const line_equation& l2)
{
    auto det = determinant(l1.a, l1.b, l2.a, l2.b);
    if (det == 0.0)
        return error_point;

    auto detx = determinant(-l1.c,  l1.b, -l2.c,  l2.b);
    auto dety = determinant( l1.a, -l1.c,  l2.a, -l2.c);
    return {detx / det, dety / det};
}

inline point get_lines_intersection(const point& p1, const point& p2, const point& q1, const point& q2)
{
    return get_lines_intersection(get_line_equation(p1, p2), get_line_equation(q1, q2));
}

inline point get_segments_intersection(const point& p1, const point& p2, const point& q1, const point& q2)
{
    auto intersection = get_lines_intersection(p1, p2, q1, q2);
    if (intersection == error_point)
        return error_point;
    
    if (point_between_segment_vertices(intersection, p1, p2) &&
        point_between_segment_vertices(intersection, q1, q2))
        return intersection;
    return error_point;
}

inline bool point_in_polygon(const std::vector<point>& vertices, const point& p)
{
    if (vertices.size() <= 2)
        return false;

    point dummy_point {p.x, 1.0};
    unsigned int no_of_intersections = 0;
    for (std::size_t idx = 1; idx < vertices.size(); ++idx)
    {
        if (get_segments_intersection(vertices[idx - 1], vertices[idx], p, dummy_point) == error_point)
            continue;

        no_of_intersections ++;
    }
    if (get_segments_intersection(vertices[0], vertices[vertices.size() - 1], p, dummy_point) != error_point)
        no_of_intersections ++;

    return (no_of_intersections % 2) == 1;
}
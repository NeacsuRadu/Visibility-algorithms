#pragma once
#include <cmath>
#include <iostream>
#include <vector>

enum class orientation: int
{
    right = -1,
    collinear,
    left
};

enum class position: unsigned int
{
    interior,
    line,
    point
};

inline int index(int idx, int size)
{
    if (idx >= 0)
        return idx % size;
    return size + (idx % size);
}

struct point
{
    double x = 0.0;
    double y = 0.0;
    bool is_dup = false;
    bool is_deg = false;
};

struct compare_pt 
{
    bool operator () (const point& a, const point& b) const 
    {
		if (a.x == b.x)
			return a.y < b.y;
        return a.x < b.x;
    }
};

struct triangle;

struct edge
{
    point a;
    point b;

    edge     * dual = nullptr;
    triangle * tri = nullptr;
};

struct triangle
{
    edge * e1 = nullptr;
    edge * e2 = nullptr;
    edge * e3 = nullptr;
};

inline triangle * get_triangle(const point& a, const point& b, const point& c)
{
    edge * e1 = new edge();
    e1->a = a;
    e1->b = b;
    
    edge * e2 = new edge();
    e2->a = b;
    e2->b = c;

    edge * e3 = new edge();
    e3->a = c;
    e3->b = a;

    triangle * tri = new triangle;
    tri->e1 = e1;
    tri->e2 = e2;
    tri->e3 = e3;

    return tri;
}

static point error_point = {-99999.0, -99999.0};
inline bool operator == (const point& p, const point& q)
{
    return p.x == q.x && p.y == q.y && p.is_dup == q.is_dup;
}

inline bool operator != (const point& p, const point& q)
{
    return !(p == q);
}

inline bool operator == (const edge& e1, const edge& e2)
{
    return e1.a == e2.a && e1.b == e2.b;
}

inline bool operator != (const edge& e1, const edge& e2)
{
    return !(e1 == e2);
}

inline double angleo(const point& p) // origin referenced
{
    const double pi = std::acos(-1);
    if (p.x > 0)
    {
        if (p.y >= 0)
            return std::atan(p.y / p.x);
        else 
            return std::atan(p.y / p.x) + 2 * pi;
    }
    else if (p.x == 0)
    {
        if (p.y > 0)
            return pi / 2;
        else if (p.y < 0)
            return 3 * pi / 2;
        else 
            throw std::runtime_error("");
    }
    else 
        return std::atan(p.y / p.x) + pi;
}

struct vertex 
{
    point coord;

};

struct line_equation
{
    /* ax + by + c = 0 */
    double a = 0;
    double b = 0;
    double c = 0;
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
        return {1, 0, -p.x};
    if (p.y == q.y)
        return {0, 1, -p.y};
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

inline bool point_is_triangle_vertex(const point& t1, const point& t2, const point& t3, const point& point)
{
    //std::cout << "point is vertex: " << point.x << " " << point.y << std::endl;
    //std::cout << t1.x << " " << t1.y << std::endl;
   // std::cout << t2.x << " " << t2.y << std::endl;
    //std::cout << t3.x << " " << t3.y << std::endl;
    return t1 == point || t2 == point || t3 == point; 
}

inline bool point_on_triangle(const point& t1, const point& t2, const point& t3, const point& point)
{
    int aux = static_cast<int>(test_orientation(t1, t2, point)) + 
              static_cast<int>(test_orientation(t2, t3, point)) + 
              static_cast<int>(test_orientation(t3, t1, point));
    return test_orientation(t1, t2, point) != orientation::right &&
           test_orientation(t2, t3, point) != orientation::right &&
           test_orientation(t3, t1, point) != orientation::right &&
           aux  == 2;
}

inline bool point_is_in_tri_interior(const point& t1, const point& t2, const point& t3, const point& p)
{
    return  test_orientation(t1, t2, p) == orientation::left &&
            test_orientation(t2, t3, p) == orientation::left &&
            test_orientation(t3, t1, p) == orientation::left;
}

inline bool point_in_triangle(const point& t1, const point& t2, const point& t3, const point& point)
{
    // counter clockwise order 
    return test_orientation(t1, t2, point) != orientation::right &&
           test_orientation(t2, t3, point) != orientation::right &&
           test_orientation(t3, t1, point) != orientation::right;
}

inline bool point_in_triangle(const point& p, const triangle * t)
{
    //std::cout << "point in triangle:" << std::endl;
    auto o1 = test_orientation(t->e1->a, t->e1->b, p);
    auto o2 = test_orientation(t->e2->a, t->e2->b, p);
    auto o3 = test_orientation(t->e3->a, t->e3->b, p);
    //std::cout << static_cast<int>(o1) << " " << static_cast<int>(o2) << " " << static_cast<int>(o3) << std::endl;
    return o1 != orientation::right && 
        o2 != orientation::right &&
        o3 != orientation::right;
}

inline bool point_in_polygon(const std::vector<point>& vertices, const point& p)
{
    //std::cout << "point in polygon" << std::endl;
    if (vertices.size() <= 2)
        return {};

    //std::cout << "Polygon: " << std::endl;
    /*for (auto& pt: vertices)
        std::cout << pt.x << " " << pt.y << " / ";
    std::cout << std::endl;*/

    point dummy_point {p.x, 2000.0};
    unsigned int no_of_intersections = 0;
    for (std::size_t idx = 1; idx < vertices.size(); ++idx)
    {
        //std::cout << "try edge: " << vertices[idx - 1].x << " " << vertices[idx - 1].y << " / " << vertices[idx].x << " " << vertices[idx].y << std::endl;
        auto inter = get_segments_intersection(vertices[idx - 1], vertices[idx], p, dummy_point);
        if (inter == error_point || inter == vertices[idx])
            continue;

        //std::cout << "edge intersects in " << inter.x << " " << inter.y << std::endl;
        no_of_intersections ++;
    }
    //std::cout << "try edge: " << vertices[0].x << " " << vertices[0].y << " / " << vertices[vertices.size() - 1].x << " " << vertices[vertices.size() - 1].y << std::endl;
    auto inter = get_segments_intersection(vertices[0], vertices[vertices.size() - 1], p, dummy_point);
    if (inter != error_point && inter != vertices[0])
    {
        //std::cout << "edge intersects in " << inter.x << " " << inter.y << std::endl;
        no_of_intersections ++;
    }

    //std::cout << "Number of intersections: " << no_of_intersections << std::endl;

    return (no_of_intersections % 2 == 1);
}
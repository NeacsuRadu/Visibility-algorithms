#include "triangulation.h"
#include "circular_list.h"
#include "polygon_helpers.h"

#include <iostream>
#include <algorithm>

triangulation * singleton<triangulation>::instance = nullptr;

/* 
    pp - vertex of polygon
    ph - vertex of hole
*/
bool bridge_intersects_polygon(const std::vector<point>& polygon, const point& pp, const point& ph)
{
    auto sz = polygon.size();
    if (pp != polygon[0] && pp != polygon[sz - 1])
    {
        auto inters = get_segments_intersection(pp, ph, polygon[0], polygon[sz - 1]);
        if (inters != error_point)
            return true;
    }

    for (std::size_t idx = 0; idx < sz - 1; idx ++)
    {
        if (pp == polygon[idx] || pp == polygon[idx + 1])
            continue;

        auto inters = get_segments_intersection(pp, ph, polygon[idx], polygon[idx + 1]);
        if (inters != error_point)
            return true;
    }

    return false;
}

bool bridge_intersects_holes(const std::vector<std::vector<point>>& holes, const point& pp, const point& ph)
{
    for (std::size_t idx = 1; idx < holes.size(); ++ idx)
        if (bridge_intersects_polygon(holes[idx], ph, pp))
            return true;

    return false;
}

void set_triangle_to_edges(triangle *&tri)
{
    tri->e1->tri = tri;
    tri->e2->tri = tri;
    tri->e3->tri = tri;
}

struct boundary_point // used for triangulation
{
    point coords;
    edge * next = nullptr;
    edge * prev = nullptr;
};

bool is_ear(node<boundary_point> *& node)
{
    auto it = node->next->next;
    while (it != node->prev)
    {
        if (it->info.coords == node->info.coords ||
            it->info.coords == node->prev->info.coords ||
            it->info.coords == node->next->info.coords)
        {
            it = it->next;
            continue;
        }
        if (point_in_triangle(node->prev->info.coords, node->info.coords, node->next->info.coords, it->info.coords))
            return false;
        it = it->next;
    }
    return true;
}


void triangulation::compute_triangulation(const std::vector<std::vector<point>>& polygons)
{
    m_triangulation.clear();
    _preprocess_polygon(polygons);

    std::vector<boundary_point> aux;
    for (auto& pt: m_degenerate_polygon)
        aux.push_back({pt});

    int size = aux.size();
    aux[size - 1].next = new edge();
    aux[size - 1].next->a = aux[size - 1].coords;
    aux[size - 1].next->b = aux[0].coords;
    for (int idx = 0; idx < size - 1; ++idx)
    {
        edge * e_next = new edge();
        e_next->a = aux[idx].coords;
        e_next->b = aux[index(idx + 1, size)].coords;

        aux[idx].next = e_next;
        aux[idx].prev = aux[index(idx - 1, size)].next;
    }
    aux[size - 1].prev = aux[size - 2].next;

    circular_list<boundary_point> list;
    for (auto it: aux)
        list.push_back(it);

    auto list_it = list.get_first();
    while (list_it->next != list.get_first())
    {
        if (list_it->info.coords.is_dup && 
            list_it->info.next->dual == nullptr)
        {
            //std::cout << "found one" << std::endl;
            auto search_it = list_it->next;
            while (true)
            {
                if (search_it->info.coords.is_dup && 
                    search_it->next->info.coords == list_it->info.coords)
                    break;
                search_it = search_it->next;
            }
            //std::cout << list_it->info.next->a.x << " " << list_it->info.next->a.y << "  " << list_it->info.next->b.x << " " << list_it->info.next->b.y << std::endl;
            //std::cout << search_it->info.next->a.x << " " << search_it->info.next->a.y << "  " << search_it->info.next->b.x << " " << search_it->info.next->b.y << std::endl;
            list_it->info.next->dual = search_it->info.next;
            search_it->info.next->dual = list_it->info.next;
        }
        list_it = list_it->next;
    }

    int no_of_triangles = size - 3;
    node<boundary_point> * it = list.get_first();
    while (no_of_triangles > 0)
    {
        //std::cout << "loop" << std::endl;
        //std::cout << "LOOP" << std::endl;
        if (test_orientation(it->prev->info.coords, it->info.coords,  it->next->info.coords) == orientation::left &&
            is_ear(it))
        {
            auto p1 = it->prev->info.coords;
            auto p2 = it->info.coords;
            auto p3 = it->next->info.coords;
            //std::cout << "found point" << p1.x << " " << p1.y << " / " << p2.x << " " << p2.y << " / " << p3.x << " " << p3.y << std::endl;  
            auto tri = new triangle();
            tri->e1 = it->info.prev;
            tri->e2 = it->info.next;
            auto e3 = new edge();
            e3->a = it->info.next->b;
            e3->b = it->info.prev->a;
            tri->e3 = e3;

            auto e3_dual = new edge();
            e3_dual->a = it->info.prev->a;
            e3_dual->b = it->info.next->b;
            e3_dual->dual = e3;
            e3->dual = e3_dual;

            it->prev->info.next = e3_dual;
            it->next->info.prev = e3_dual;

            set_triangle_to_edges(tri);

            it = list.remove(it);

            m_triangulation.push_back(tri);
            -- no_of_triangles;
            continue;
        }

        it = it->next;
    }

    auto tri = new triangle();
    tri->e1 = it->info.next;
    tri->e2 = it->next->info.next;
    tri->e3 = it->info.prev;
    set_triangle_to_edges(tri);
    m_triangulation.push_back(tri);
}

void triangulation::_preprocess_polygon(const std::vector<std::vector<point>>& p)
{
    /* counter clockwise order for the polygon */
    m_polygons = p;
    if (!is_counter_clockwise(m_polygons[0]))
        revert_order(m_polygons[0]);

    /* clockwise order for the holes */
    for (std::size_t idx = 1; idx < m_polygons.size(); ++ idx)
        if (is_counter_clockwise(m_polygons[idx]))
            revert_order(m_polygons[idx]);

    _compute_degenerate_polygon();
}

void triangulation::_compute_degenerate_polygon()
{
    m_degenerate_polygon = m_polygons[0];
    if (m_polygons.size() == 1)
        return; // there are no holes 

    auto no_of_iterations = m_polygons.size();
    for (std::size_t _ = 1; _ < no_of_iterations; ++ _) // we need to link each hole to the degenerate polygon
    {
        for (std::size_t crr_hole = 1; crr_hole < m_polygons.size(); ++ crr_hole)
        {
            auto& hole = m_polygons[crr_hole];

            /* find all potential bridge edges */
            std::vector<bridge_edge> bridge_edges;
            for (std::size_t idx_dp = 0; idx_dp < m_degenerate_polygon.size(); ++ idx_dp)
                for (std::size_t idx_h = 0; idx_h < hole.size(); ++ idx_h)
                    bridge_edges.push_back({idx_dp, idx_h,
                        distance(m_degenerate_polygon[idx_dp], hole[idx_h])});

            /* sort all bridge edges ascending using the distance */
            std::sort(bridge_edges.begin(), bridge_edges.end(),
                [](const bridge_edge& e1, const bridge_edge& e2) -> bool {
                    return e1.distance < e2.distance;
                });


            /* find the shortest bridge edge that does not intersect the polygon or other holes */
            std::size_t idx_edge = 0;
            for (const auto& e: bridge_edges)
            {
                if (!bridge_intersects_polygon(m_degenerate_polygon, m_degenerate_polygon[e.index_polygon], hole[e.index_hole]) &&
                    !bridge_intersects_holes(m_polygons, m_degenerate_polygon[e.index_polygon], hole[e.index_hole]))
                    break;
                ++ idx_edge;
            }

            if (idx_edge == bridge_edges.size())
                continue;

            auto& e = bridge_edges[idx_edge];
            std::vector<point> aux;
            aux.insert(aux.end(), m_degenerate_polygon.begin(), m_degenerate_polygon.begin() + e.index_polygon + 1);
            aux[aux.size() - 1].is_dup = true;
            aux.insert(aux.end(), hole.begin() + e.index_hole, hole.end());
            aux.insert(aux.end(), hole.begin(), hole.begin() + e.index_hole + 1);
            aux[aux.size() - 1].is_dup = true;
            aux.insert(aux.end(), m_degenerate_polygon.begin() + e.index_polygon, m_degenerate_polygon.end());

            m_degenerate_polygon.swap(aux);

            if (crr_hole < m_polygons.size() - 1)
                m_polygons[crr_hole].swap(m_polygons[m_polygons.size() - 1]);
            m_polygons.pop_back();
        }
    }
}
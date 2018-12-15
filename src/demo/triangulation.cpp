#include "triangulation.h"
#include "circular_list.h"

#include <iostream>
struct boundary_point
{
    point coords;
    edge * next = nullptr;
    edge * prev = nullptr;
};

void set_triangle_to_edges(triangle *&tri)
{
    tri->e1->tri = tri;
    tri->e2->tri = tri;
    tri->e3->tri = tri;
}

bool is_ear(node<boundary_point> *& node)
{
    auto it = node->next->next;
    while (it != node->prev)
    {
        if (point_in_triangle(node->prev->info.coords, node->info.coords, node->next->info.coords, it->info.coords))
            return false;
        it = it->next;
    }
    return true;
}

std::vector<triangle*> get_triangulation(const std::vector<point>& points)
{
    std::cout << "Get triangulation" << std::endl;
    std::vector<boundary_point> aux;
    for (auto& pt: points)
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

    int no_of_triangles = size - 3;
    node<boundary_point> * it = list.get_first();
    std::vector<triangle*> triangles;
    while (no_of_triangles > 0)
    {
        std::cout << "LOOP" << std::endl;
        if (test_orientation(it->prev->info.coords, it->info.coords,  it->next->info.coords) == orientation::left &&
            is_ear(it))
        {
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

             triangles.push_back(tri);
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
    triangles.push_back(tri);

    return triangles;
}
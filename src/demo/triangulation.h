#pragma once
#include "singleton.h"
#include "geometry.h"

#include <vector>

class triangulation: 
    public singleton<triangulation>
{
public:
    inline std::vector<triangle*> get_triangulation() const { return m_triangulation; }
    inline std::vector<point> get_degenerate_polygon() const { return m_degenerate_polygon; }

    void compute_triangulation(const std::vector<std::vector<point>>& polygons);
private:
    void _preprocess_polygon(const std::vector<std::vector<point>>& polygons);
    void _compute_degenerate_polygon();

    std::vector<point>               m_degenerate_polygon;
    std::vector<std::vector<point>>  m_polygons; // m_polygons[0] is the polygon and the other ones are the holes
    std::vector<triangle*>           m_triangulation; // vertices have counter clockwise order 

    struct bridge_edge // used to compute degenerate polygon
    {
        std::size_t index_polygon;
        std::size_t index_hole;
        double distance = 0.0;
    };
};

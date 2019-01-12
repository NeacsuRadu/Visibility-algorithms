#pragma once
#include "geometry.h"
#include <queue>

class roboto
{
public:
    void set_path(const std::vector<point>& path);
    void update();

    inline bool can_render() const { return m_velocity.x != 0.0 && m_velocity.y != 0.0; }
    inline point get_position() const { return m_position; }
private:
    void _compute_velocity();

    std::queue<point> m_path;
    point m_position;
    point m_velocity = {0.0, 0.0};
    double m_rem_distance = 0.0;
};
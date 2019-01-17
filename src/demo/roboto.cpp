#include "roboto.h"

void roboto::set_path(const std::vector<point>& path)
{
    for (const auto& pt: path)
        m_path.push(pt);

    m_position = m_path.front();
    m_path.pop();

    _compute_velocity();
}

void roboto::update()
{
    if (m_path.empty())
        return; 

    point upd = {m_position.x + m_velocity.x, m_position.y + m_velocity.y};
    double dist = distance(upd, m_position);

    if (dist >= m_rem_distance)
    {
        m_position = m_path.front();
        m_path.pop();

        _compute_velocity();
    }
    else 
    {
        m_position = upd;
        m_rem_distance -= dist;
    }
}

void roboto::_compute_velocity()
{
    if (m_path.empty())
    {
        m_velocity = { 0.0, 0.0 };
        m_rem_distance = 0.0;
        return;
    }

    auto& pt = m_path.front();
    point diff;
    diff.x = pt.x - m_position.x;
    diff.y = pt.y - m_position.y;

    m_rem_distance = distance(m_position, pt);
    //std::cout << "Distance: "  << m_rem_distance << std::endl;
    if (m_rem_distance == 0.0)
        m_rem_distance = 2.0;

    m_velocity.x = diff.x / (m_rem_distance * 5);
    m_velocity.y = diff.y / (m_rem_distance * 5);
}